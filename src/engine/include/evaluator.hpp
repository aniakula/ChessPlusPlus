#pragma once

#include "board.hpp"
#include "types.hpp"
#include <array>

namespace chesspp::engine {

struct EvaluationConfig {
  bool use_piece_square_tables{true};
  bool use_mobility{false};
  bool use_king_safety{false};
};

// position based score bonuses:
// pawn, knight, bishop, rook, queen (in that order)
inline constexpr const std::array<std::array<core::Score, 64>, 5>
    position_tables{};

// score per material piece
// order: pawn=1, knight=3, bishop=4, rook=5, queen=9
inline constexpr const std::array<core::Score, 6> material_table{1, 3, 4, 5, 9};

class Evaluator {
public:
  explicit Evaluator(EvaluationConfig config = {});

  // Score from White's point of view. Search can multiply/negate based on side
  // to move. Keep this allocation-free and branch-light.
  [[nodiscard]] chesspp::core::Score
  evaluate(const chesspp::core::Board &board) const noexcept;

  [[nodiscard]] chesspp::core::Score
  material_score(const chesspp::core::Board &board) const noexcept;

  [[nodiscard]] chesspp::core::Score
  piece_square_score(const chesspp::core::Board &board) const noexcept;

private:
  EvaluationConfig config_{};
};

} // namespace chesspp::engine
