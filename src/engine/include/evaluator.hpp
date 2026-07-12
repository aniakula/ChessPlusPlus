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

inline constexpr std::array<std::array<core::Weight, 64>, 6> position_tables{
    core::PAWN_POSITION_SCORE,   core::KNIGHT_POSITION_SCORE,
    core::BISHOP_POSITION_SCORE, core::ROOK_POSITION_SCORE,
    core::QUEEN_POSITION_SCORE,  core::KING_POSITION_SCORE};

inline constexpr std::array<core::Score, 6> material_table{
    core::PAWN_SCORE, core::KNIGHT_SCORE, core::BISHOP_SCORE, core::ROOK_SCORE,
    core::QUEEN_SCORE};

class Evaluator {
public:
  explicit Evaluator(EvaluationConfig config = {});

  // Score from White's point of view. Search can multiply/negate based on side
  // to move. Keep this allocation-free and branch-light.
  [[nodiscard]] chesspp::core::Score
  evaluate(const chesspp::core::Board &board) const noexcept;

  [[nodiscard]] chesspp::core::Score
  material_score(const chesspp::core::Board &board,
                 const core::Color side) const noexcept;

  [[nodiscard]] chesspp::core::Score
  positional_score(const chesspp::core::Board &board,
                   const core::Color side) const noexcept;

  [[nodiscard]] chesspp::core::Score
  advantages_score(const chesspp::core::Board &board,
                   const core::Color side) const noexcept;

private:
  EvaluationConfig config_{};
};

} // namespace chesspp::engine
