#include "search.hpp"

namespace chesspp::engine {

namespace {

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
  // table_size must be a power of two. resize() guarantees that.
  return static_cast<std::size_t>(key) & (table_size - 1ULL);
}

} // namespace

TranspositionTable::TranspositionTable(std::size_t megabytes) {
  resize(megabytes);
}

void TranspositionTable::resize(std::size_t megabytes) {
  // This is the cache allocation point. Call this from Engine setup/options,
  // not inside alpha_beta. Resizing during search invalidates cached entries
  // and is much too expensive for the hot path.
  entries_.assign(bytes_to_entry_count(megabytes), TranspositionEntry{});
}

void TranspositionTable::clear() noexcept {
  // Clears cached search results while preserving allocated memory.
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

  // The index is only a bucket. Always verify full key to reject collisions.
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

  // Initial replacement policy: always replace. Later, prefer deeper entries
  // or add a generation counter to avoid useful old entries being overwritten.
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
  // TODO: Iterative deepening loop that calls alpha_beta().
  chesspp::core::MoveList root_moves;
  move_orderer_.score_moves(board, root_moves);
  (void)limits;
  return {};
}

void Search::clear_cache() noexcept { transposition_table_.clear(); }

void Search::resize_cache(std::size_t megabytes) {
  transposition_table_.resize(megabytes);
}

chesspp::core::Score Search::alpha_beta(chesspp::core::Board &board, int depth,
                                        chesspp::core::Score alpha,
                                        chesspp::core::Score beta, int ply) {
  // TODO: Probe TT before generating moves:
  // TranspositionEntry entry;
  // if (transposition_table_.probe(board.zobrist_key(), entry) &&
  //     entry.depth >= depth) {
  //   Use entry.bound to tighten alpha/beta or return exact score.
  // }
  //
  // TODO: Store after searching:
  // TranspositionEntry result{board.zobrist_key(), best_move, best_score,
  //                          depth, computed_bound};
  // transposition_table_.store(result);
  (void)board;
  (void)depth;
  (void)alpha;
  (void)beta;
  (void)ply;
  return 0;
}

chesspp::core::Score Search::quiescence(chesspp::core::Board &board,
                                        chesspp::core::Score alpha,
                                        chesspp::core::Score beta, int ply) {
  // TODO: Search only tactical moves to avoid horizon effect.
  (void)board;
  (void)alpha;
  (void)beta;
  (void)ply;
  return 0;
}

bool Search::should_stop(const SearchLimits &limits) const noexcept {
  // TODO: Check node/time limits.
  (void)limits;
  return false;
}

chesspp::core::Score Search::terminal_score(const chesspp::core::Board &board,
                                            int ply) const noexcept {
  // TODO: Return mate score adjusted by ply or draw score.
  (void)board;
  (void)ply;
  return 0;
}

} // namespace chesspp::engine
