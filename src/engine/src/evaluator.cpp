#include "evaluator.hpp"

namespace chesspp::engine {

Evaluator::Evaluator(EvaluationConfig config) : config_{config} {}

chesspp::core::Score Evaluator::evaluate(
    const chesspp::core::Board& board) const noexcept {
  // TODO: Combine material, PST, mobility, king safety, pawn structure, etc.
  (void)board;
  return 0;
}

chesspp::core::Score Evaluator::material_score(
    const chesspp::core::Board& board) const noexcept {
  // TODO: Popcount each piece bitboard and multiply by piece values.
  (void)board;
  return 0;
}

chesspp::core::Score Evaluator::piece_square_score(
    const chesspp::core::Board& board) const noexcept {
  // TODO: Iterate piece bitboards and add piece-square table bonuses.
  (void)board;
  return 0;
}

} // namespace chesspp::engine
