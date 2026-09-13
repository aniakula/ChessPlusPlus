#include "search.hpp"
#include "types.hpp"

#include <algorithm>
#include <chrono>

namespace chesspp::engine {

namespace {

// Quiescence can follow long checking sequences even though the normal search
// depth has reached zero. This guard protects the stack in pathological
// positions. It is deliberately much larger than normal search depths.
constexpr int MAX_SEARCH_PLY = 128;

// Mate scores stored in the TT need special handling because a mate score
// contains its distance from the current search root. The same position may be
// reached at a different ply later, so remove the current ply when storing and
// add the new ply when probing.
constexpr chesspp::core::Score MATE_TT_THRESHOLD =
    chesspp::core::MATE_SCORE - MAX_SEARCH_PLY;

[[nodiscard]] chesspp::core::Score score_to_tt(chesspp::core::Score score,
                                               int ply) noexcept {
  if (score >= MATE_TT_THRESHOLD) {
    return score + ply;
  }
  if (score <= -MATE_TT_THRESHOLD) {
    return score - ply;
  }
  return score;
}

[[nodiscard]] chesspp::core::Score score_from_tt(chesspp::core::Score score,
                                                 int ply) noexcept {
  if (score >= MATE_TT_THRESHOLD) {
    return score - ply;
  }
  if (score <= -MATE_TT_THRESHOLD) {
    return score + ply;
  }
  return score;
}

[[nodiscard]] bool is_tactical(chesspp::core::Move move) noexcept {
  const chesspp::core::MoveFlag flags = move.flags();
  return chesspp::core::has_flag(flags, chesspp::core::MoveFlag::Capture) ||
         chesspp::core::has_flag(flags, chesspp::core::MoveFlag::EnPassant) ||
         chesspp::core::has_flag(flags, chesspp::core::MoveFlag::Promotion);
}

[[nodiscard]] std::size_t bytes_to_entry_count(std::size_t megabytes) {
  const std::size_t bytes = megabytes * 1024ULL * 1024ULL;
  const std::size_t requested = bytes / sizeof(TranspositionEntry);

  // Keep at least one slot so probe/store can be simple.
  std::size_t count = 1;
  while (count < requested) {
    count <<= 1ULL;
  }
  return count;
}

[[nodiscard]] std::size_t table_index(chesspp::core::HashKey key,
                                      std::size_t table_size) {
  // table_size must be a power of two
  // only use bottom bits to index (22 bits for 64MB)
  return static_cast<std::size_t>(key) & (table_size - 1ULL);
}

} // namespace

TranspositionTable::TranspositionTable(std::size_t megabytes) {
  resize(megabytes);
}

void TranspositionTable::resize(std::size_t megabytes) {
  // Resizing during search invalidates cached entries
  // Only call during setup/testing
  entries_.assign(bytes_to_entry_count(megabytes), TranspositionEntry{});
}

void TranspositionTable::clear() noexcept {
  // Clear cached search results & preserve allocated memory
  for (TranspositionEntry &entry : entries_) {
    entry = {};
  }
}

bool TranspositionTable::probe(chesspp::core::HashKey key,
                               TranspositionEntry &out) const noexcept {
  if (entries_.empty()) {
    return false;
  }

  const TranspositionEntry &entry = entries_[table_index(key, entries_.size())];

  // verify full key to reject collisions.
  if (entry.key != key) {
    return false;
  }

  out = entry;
  return true;
}

void TranspositionTable::store(const TranspositionEntry &entry) noexcept {
  if (entries_.empty()) {
    return;
  }

  TranspositionEntry &slot = entries_[table_index(entry.key, entries_.size())];

  // always replace. Later, prefer deeper entries
  slot = entry;
}

void MoveOrderer::score_moves(const chesspp::core::Board &board,
                              chesspp::core::MoveList &moves,
                              chesspp::core::Move transposition_move) noexcept {
  // TODO: Score TT move first, then captures/promotions/killers/history.
  (void)board;
  (void)moves;
  (void)transposition_move;
}

Search::Search(Evaluator evaluator, TranspositionTable transposition_table)
    : evaluator_{evaluator}, transposition_table_{transposition_table} {}

SearchResult Search::search(chesspp::core::Board &board,
                            const SearchLimits &limits) {
  // Everything below uses one clock and one set of limits for this complete
  // "think". Recursive functions read active_limits_ rather than restarting
  // the timer at every node.
  stats_ = {};
  search_start_ = std::chrono::steady_clock::now();
  active_limits_ = limits;
  stopped_ = false;

  chesspp::core::MoveList root_moves;
  chesspp::core::MoveGenerator::generate_legal(board, root_moves);

  SearchResult result{};
  if (root_moves.empty()) {
    // No legal move means checkmate or stalemate. No recursive search is
    // needed, and best_move intentionally remains null.
    result.score = terminal_score(board, 0);
    result.stats = stats_;
    return result;
  }

  // Always retain a legal fallback. If a zero/tiny time limit interrupts the
  // first iteration, Engine still receives a legal move rather than null.
  result.best_move = root_moves[0];
  const chesspp::core::Score white_score = evaluator_.evaluate(board);
  result.score = board.side_to_move() == chesspp::core::Color::White
                     ? white_score
                     : -white_score;

  // Iterative deepening completes depth 1 before trying depth 2, and so on.
  // Apart from producing a usable result under time pressure, each completed
  // iteration seeds the TT with moves that improve later move ordering.
  const int maximum_depth = std::max(0, limits.max_depth);
  for (int depth = 1; depth <= maximum_depth; ++depth) {
    if (should_stop(limits)) {
      stopped_ = true;
      break;
    }

    const chesspp::core::Score score =
        alpha_beta(board, depth, -chesspp::core::INF, chesspp::core::INF, 0);

    // A stopped iteration is incomplete: some root moves may not have been
    // searched. Keep the result from the last fully completed depth.
    if (stopped_) {
      break;
    }

    TranspositionEntry root_entry{};
    if (transposition_table_.probe(board.zobrist_key(), root_entry) &&
        !root_entry.best_move.is_null() &&
        root_moves.contains(root_entry.best_move)) {
      result.best_move = root_entry.best_move;
    }
    result.score = score;
    result.depth_reached = depth;
  }

  result.stats = stats_;
  return result;
}

void Search::clear_cache() noexcept { transposition_table_.clear(); }

void Search::resize_cache(std::size_t megabytes) {
  transposition_table_.resize(megabytes);
}

chesspp::core::Score Search::alpha_beta(chesspp::core::Board &board, int depth,
                                        chesspp::core::Score alpha,
                                        chesspp::core::Score beta, int ply) {
  // This is a negamax search: every returned score is from the point of view
  // of the side to move at this node. After making a move, the opponent is to
  // move, so the recursive result is negated.
  if (should_stop(active_limits_)) {
    stopped_ = true;
    return 0;
  }

  if (depth <= 0) {
    return quiescence(board, alpha, beta, ply);
  }

  ++stats_.nodes;

  // Beyond this safety limit, use a static evaluation rather than risking an
  // excessively deep call stack.
  if (ply >= MAX_SEARCH_PLY) {
    const chesspp::core::Score white_score = evaluator_.evaluate(board);
    return board.side_to_move() == chesspp::core::Color::White ? white_score
                                                               : -white_score;
  }

  TranspositionEntry cached{};
  chesspp::core::Move transposition_move = chesspp::core::Move::null();

  // A TT entry can be used as a score only if it was searched at least as
  // deeply as this request. A shallower entry is still valuable for ordering:
  // its best move is likely worth searching first.
  if (transposition_table_.probe(board.zobrist_key(), cached)) {
    ++stats_.transposition_hits;
    transposition_move = cached.best_move;

    if (cached.depth >= depth) {
      const chesspp::core::Score cached_score =
          score_from_tt(cached.score, ply);
      if (cached.bound == BoundType::Exact) {
        return cached_score;
      }
      if (cached.bound == BoundType::LowerBound) {
        alpha = std::max(alpha, cached_score);
      } else {
        beta = std::min(beta, cached_score);
      }
      if (alpha >= beta) {
        return cached_score;
      }
    }
  }

  // Save the effective window after applying any TT bounds. This is the
  // window the moves below are actually searched against, so it determines
  // whether the result is exact or merely a bound.
  const chesspp::core::Score search_alpha = alpha;
  const chesspp::core::Score search_beta = beta;

  chesspp::core::MoveList moves;
  chesspp::core::MoveGenerator::generate_legal(board, moves);
  if (moves.empty()) {
    return terminal_score(board, ply);
  }

  // score_moves is currently a hook; once it reorders MoveList, the TT move
  // and captures should be placed first without changing search correctness.
  move_orderer_.score_moves(board, moves, transposition_move);

  chesspp::core::Score best_score = -chesspp::core::INF;
  chesspp::core::Move best_move = chesspp::core::Move::null();

  for (const chesspp::core::Move move : moves) {
    chesspp::core::UndoState undo{};
    board.make_move(move, undo);
    const chesspp::core::Score score =
        -alpha_beta(board, depth - 1, -beta, -alpha, ply + 1);
    board.unmake_move(undo);

    // Always restore the board before propagating a stop. Do not store partial
    // results in the TT because an unsearched sibling might be the best move.
    if (stopped_) {
      return 0;
    }

    if (score > best_score) {
      best_score = score;
      best_move = move;
    }
    alpha = std::max(alpha, score);

    if (alpha >= beta) {
      ++stats_.beta_cutoffs;
      break;
    }
  }

  // Classify what this search proved relative to the caller's original
  // window. Fail-low is an upper bound, fail-high is a lower bound, and a
  // score inside the window is exact.
  BoundType bound = BoundType::Exact;
  if (best_score <= search_alpha) {
    bound = BoundType::UpperBound;
  } else if (best_score >= search_beta) {
    bound = BoundType::LowerBound;
  }

  transposition_table_.store({board.zobrist_key(), best_move,
                              score_to_tt(best_score, ply),
                              static_cast<std::int16_t>(depth), bound});
  return best_score;
}

chesspp::core::Score Search::quiescence(chesspp::core::Board &board,
                                        chesspp::core::Score alpha,
                                        chesspp::core::Score beta, int ply) {
  if (should_stop(active_limits_)) {
    stopped_ = true;
    return 0;
  }
  ++stats_.nodes;

  const bool in_check = board.in_check(board.side_to_move());
  chesspp::core::MoveList moves;
  chesspp::core::MoveGenerator::generate_legal(board, moves);

  // Checking terminal state here matters: depth can reach zero on the exact
  // move that delivers mate or stalemate.
  if (moves.empty()) {
    return terminal_score(board, ply);
  }

  const chesspp::core::Score white_score = evaluator_.evaluate(board);
  const chesspp::core::Score stand_pat =
      board.side_to_move() == chesspp::core::Color::White ? white_score
                                                          : -white_score;

  if (ply >= MAX_SEARCH_PLY) {
    return stand_pat;
  }

  // "Stand pat" means assuming the side to move makes no tactical move. It is
  // legal only when not in check; a checked king must search every legal
  // evasion rather than accepting a static evaluation.
  if (!in_check) {
    if (stand_pat >= beta) {
      ++stats_.beta_cutoffs;
      return stand_pat;
    }
    alpha = std::max(alpha, stand_pat);
  }

  for (const chesspp::core::Move move : moves) {
    // In quiet positions, extend only unstable moves: captures and promotions.
    // In check, every legal move is searched because quiet king/blocking moves
    // may be the only ways to escape.
    if (!in_check && !is_tactical(move)) {
      continue;
    }

    chesspp::core::UndoState undo{};
    board.make_move(move, undo);
    const chesspp::core::Score score =
        -quiescence(board, -beta, -alpha, ply + 1);
    board.unmake_move(undo);

    if (stopped_) {
      return 0;
    }
    if (score >= beta) {
      ++stats_.beta_cutoffs;
      return score;
    }
    alpha = std::max(alpha, score);
  }

  return alpha;
}

bool Search::should_stop(const SearchLimits &limits) const noexcept {
  if (limits.max_nodes > 0 && stats_.nodes >= limits.max_nodes) {
    return true;
  }

  if (limits.max_time_ms > 0) {
    const auto elapsed = std::chrono::steady_clock::now() - search_start_;
    const auto elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    if (static_cast<std::uint64_t>(elapsed_ms) >= limits.max_time_ms) {
      return true;
    }
  }

  return false;
}

chesspp::core::Score Search::terminal_score(const chesspp::core::Board &board,
                                            int ply) const noexcept {
  if (board.in_check(board.side_to_move())) {
    return -(core::MATE_SCORE - ply);
  }

  return 0;
}

} // namespace chesspp::engine
