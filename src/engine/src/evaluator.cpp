#include "evaluator.hpp"
#include "types.hpp"
#include <array>
#include <cstdint>

namespace chesspp::engine {

Evaluator::Evaluator(EvaluationConfig config) : config_{config} {}

chesspp::core::Score
Evaluator::evaluate(const chesspp::core::Board &board) const noexcept {
  core::Score final_score = material_score(board, core::Color::White) -
                            material_score(board, core::Color::Black);
  final_score += positional_score(board, core::Color::White) -
                 positional_score(board, core::Color::Black);
  final_score += advantages_score(board, core::Color::White) -
                 advantages_score(board, core::Color::Black);
  return final_score;
}

chesspp::core::Score
Evaluator::material_score(const chesspp::core::Board &board,
                          const core::Color side) const noexcept {
  core::Score mat_score = 0;

  std::array<core::Bitboard, 5> board_by_piece{
      board.pieces(side, core::PieceType::Pawn),
      board.pieces(side, core::PieceType::Knight),
      board.pieces(side, core::PieceType::Bishop),
      board.pieces(side, core::PieceType::Rook),
      board.pieces(side, core::PieceType::Queen)};

  for (size_t pieceInd = 0; pieceInd < board_by_piece.size(); pieceInd++) {
    while (board_by_piece[pieceInd]) {
      board_by_piece[pieceInd] =
          (board_by_piece[pieceInd] & (board_by_piece[pieceInd] - 1));
      mat_score += material_table[pieceInd];
    }
  }

  return mat_score;
}

chesspp::core::Score
Evaluator::positional_score(const chesspp::core::Board &board,
                            const core::Color side) const noexcept {

  core::Score position_score = 0;
  uint8_t colorInd = side == core::Color::White ? 0 : 1;
  const core::Phase phase = board.curr_phase();

  std::array<core::Bitboard, 6> board_by_piece{
      board.pieces(side, core::PieceType::Pawn),
      board.pieces(side, core::PieceType::Knight),
      board.pieces(side, core::PieceType::Bishop),
      board.pieces(side, core::PieceType::Rook),
      board.pieces(side, core::PieceType::Queen),
      board.pieces(side, core::PieceType::King)};

  for (size_t pieceInd = 0; pieceInd < board_by_piece.size(); pieceInd++) {
    while (board_by_piece[pieceInd]) {
      core::Square position = core::pop_lsb(board_by_piece[pieceInd]);
      position_score +=
          core::taper(position_tables[colorInd][pieceInd][position], phase);
    }
  }

  return position_score;
}

[[nodiscard]] chesspp::core::Score
Evaluator::advantages_score(const chesspp::core::Board &board,
                            const core::Color side) const noexcept {
  core::Score bonus = 0;
  // Bishop pair:
  core::Bitboard bishops = board.pieces(side, core::PieceType::Bishop);
  if (core::pop_lsb(bishops)) {
    bonus += core::taper(core::BISHOP_PAIR_WEIGHT, board.curr_phase());
  }

  return bonus;
}

} // namespace chesspp::engine
