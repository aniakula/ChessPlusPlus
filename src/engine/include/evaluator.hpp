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

inline constexpr std::array<std::array<std::array<core::Weight, 64>, 6>, 2>
    position_tables{
        {{core::WHITE_PAWN_POSITION_SCORE, core::WHITE_KNIGHT_POSITION_SCORE,
          core::WHITE_BISHOP_POSITION_SCORE, core::WHITE_ROOK_POSITION_SCORE,
          core::WHITE_QUEEN_POSITION_SCORE, core::WHITE_KING_POSITION_SCORE},
         {core::BLACK_PAWN_POSITION_SCORE, core::BLACK_KNIGHT_POSITION_SCORE,
          core::BLACK_BISHOP_POSITION_SCORE, core::BLACK_ROOK_POSITION_SCORE,
          core::BLACK_QUEEN_POSITION_SCORE, core::BLACK_KING_POSITION_SCORE}}};

inline constexpr std::array<core::Score, 6> material_table{
    core::PAWN_SCORE, core::KNIGHT_SCORE, core::BISHOP_SCORE, core::ROOK_SCORE,
    core::QUEEN_SCORE};

class Evaluator {
public:
  explicit Evaluator(EvaluationConfig config = {});

  // Score from White's point of view. Search negates based on side
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
