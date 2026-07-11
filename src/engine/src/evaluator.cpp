#include "evaluator.hpp"
#include "types.hpp"
#include <array>

namespace chesspp::engine {

Evaluator::Evaluator(EvaluationConfig config) : config_{config} {}

chesspp::core::Score
Evaluator::evaluate(const chesspp::core::Board &board) const noexcept {

  return material_score(board) + piece_square_score(board);
}

chesspp::core::Score
Evaluator::material_score(const chesspp::core::Board &board) const noexcept {
  core::Score mat_score = 0;
  core::Color us = board.side_to_move();
  // TODO: Iterate piece bitboards and add piece-square table bonuses.
  std::array<core::Bitboard, 5> board_by_piece{
      board.pieces(us, core::PieceType::Pawn),
      board.pieces(us, core::PieceType::Knight),
      board.pieces(us, core::PieceType::Bishop),
      board.pieces(us, core::PieceType::Rook),
      board.pieces(us, core::PieceType::Queen)};

  for (size_t pieceInd = 0; pieceInd < board_by_piece.size(); pieceInd++) {
    while (board_by_piece[pieceInd]) {
      board_by_piece[pieceInd] =
          (board_by_piece[pieceInd] & (board_by_piece[pieceInd] - 1));
      mat_score += material_table[pieceInd];
    }
  }

  return mat_score;
}

chesspp::core::Score Evaluator::piece_square_score(
    const chesspp::core::Board &board) const noexcept {

  core::Score position_score = 0;
  core::Color us = board.side_to_move();

  std::array<core::Bitboard, 5> board_by_piece{
      board.pieces(us, core::PieceType::Pawn),
      board.pieces(us, core::PieceType::Knight),
      board.pieces(us, core::PieceType::Bishop),
      board.pieces(us, core::PieceType::Rook),
      board.pieces(us, core::PieceType::Queen)};

  for (size_t pieceInd = 0; pieceInd < board_by_piece.size(); pieceInd++) {
    while (board_by_piece[pieceInd]) {
      core::Square position = core::pop_lsb(board_by_piece[pieceInd]);
      position_score += position_tables[pieceInd][position];
    }
  }

  return us == core::Color::White ? position_score : (-1 * position_score);
}

} // namespace chesspp::engine
