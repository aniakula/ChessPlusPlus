#pragma once

#include "board.hpp"
#include "evaluator.hpp"
#include "movegen.hpp"
#include "types.hpp"

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <vector>

namespace chesspp::engine {

enum class BoundType : std::uint8_t {
  Exact,
  LowerBound,
  UpperBound,
};

struct TranspositionEntry {
  chesspp::core::HashKey key{0};
  chesspp::core::Move best_move{};
  chesspp::core::Score score{0};
  std::int16_t depth{0};
  BoundType bound{BoundType::Exact};
};

class TranspositionTable {
public:
  explicit TranspositionTable(std::size_t megabytes = 64);

  void resize(std::size_t megabytes);
  void clear() noexcept;

  [[nodiscard]] bool probe(chesspp::core::HashKey key,
                           TranspositionEntry& out) const noexcept;
  void store(const TranspositionEntry& entry) noexcept;

private:
  // The transposition table is the engine's position cache.
  //
  // Where it lives:
  // - Keep it in engine/search, not core/board. Board only produces a
  //   zobrist_key(); Search decides how to cache search results.
  //
  // How lookup works:
  // - entries_ is a flat array for cache locality.
  // - Use board.zobrist_key() to compute an index.
  // - If entries_[index].key equals the current key, it is a table hit.
  //
  // Sizing:
  // - Prefer a power-of-two entry count so index = key & (entries_.size() - 1)
  //   instead of key % entries_.size().
  //
  // Replacement policy to implement later:
  // - Empty slot: store immediately.
  // - Same key: replace with newer/deeper result.
  // - Different key: usually replace if new entry is deeper or from newer
  //   search generation. Add an age/generation field if you want stronger TT.
  std::vector<TranspositionEntry> entries_{};
};

struct SearchLimits {
  int max_depth{5};
  std::uint64_t max_nodes{0};
  std::uint64_t max_time_ms{0};
};

struct SearchStats {
  std::uint64_t nodes{0};
  std::uint64_t transposition_hits{0};
  std::uint64_t beta_cutoffs{0};
};

inline std::ostream &operator<<(std::ostream &out, const SearchStats &stats) {
  out << "SearchStats{nodes=" << stats.nodes
      << ", transposition_hits=" << stats.transposition_hits
      << ", beta_cutoffs=" << stats.beta_cutoffs << '}';
  return out;
}

struct SearchResult {
  chesspp::core::Move best_move{};
  chesspp::core::Score score{0};
  int depth_reached{0};
  SearchStats stats{};
};

class MoveOrderer {
public:
  // Fast move ordering is often more important than raw move generation speed
  // for alpha-beta. Start with captures/promotions, then add TT move, killers,
  // history heuristic, MVV-LVA, etc.
  void score_moves(const chesspp::core::Board& board,
                   chesspp::core::MoveList& moves,
                   chesspp::core::Move transposition_move =
                       chesspp::core::Move::null()) noexcept;
};

class Search {
public:
  explicit Search(Evaluator evaluator = Evaluator{},
                  TranspositionTable transposition_table =
                      TranspositionTable{});

  [[nodiscard]] SearchResult search(chesspp::core::Board& board,
                                    const SearchLimits& limits);

  void clear_cache() noexcept;
  void resize_cache(std::size_t megabytes);

private:
  Evaluator evaluator_{};
  TranspositionTable transposition_table_{};
  MoveOrderer move_orderer_{};
  SearchStats stats_{};

  [[nodiscard]] chesspp::core::Score
  alpha_beta(chesspp::core::Board& board, int depth,
             chesspp::core::Score alpha, chesspp::core::Score beta,
             int ply);

  [[nodiscard]] chesspp::core::Score
  quiescence(chesspp::core::Board& board, chesspp::core::Score alpha,
             chesspp::core::Score beta, int ply);

  [[nodiscard]] bool should_stop(const SearchLimits& limits) const noexcept;
  [[nodiscard]] chesspp::core::Score terminal_score(
      const chesspp::core::Board& board, int ply) const noexcept;
};

}  // namespace chesspp::engine
