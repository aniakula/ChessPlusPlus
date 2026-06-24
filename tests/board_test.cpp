#include "board.hpp"
#include "types.hpp"

#include <gtest/gtest.h>

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>

namespace chesspp::core {
namespace {

Square square_from(int file, int rank) {
  return static_cast<Square>((rank * 8) + file);
}

Square square_from_algebraic(char file, char rank) {
  return square_from(static_cast<int>(file - 'a'),
                     static_cast<int>(rank - '1'));
}

void expect_piece_at(const Board &board, Square square, Color color,
                     PieceType piece) {
  const auto found = board.piece_and_color_on(square);
  ASSERT_TRUE(found.has_value())
      << "Expected a piece on square " << static_cast<int>(square);
  EXPECT_EQ(found->first, color);
  EXPECT_EQ(found->second, piece);
}

void expect_empty_at(const Board &board, Square square) {
  EXPECT_FALSE(board.piece_on(square).has_value());
  EXPECT_FALSE(board.color_on(square).has_value());
  EXPECT_FALSE(board.piece_and_color_on(square).has_value());
}

std::string log_board_to_string(const Board &board) {
  std::ostringstream out;
  log_board(board, out, true);
  return out.str();
}

Board board_from_fen(const char *fen) {
  char buffer[128]{};
  std::snprintf(buffer, sizeof(buffer), "%s", fen);
  return Board::from_fen(buffer);
}

} // namespace

TEST(BoardTest, ClearLeavesEmptyBoard) {
  Board board;
  board.set_piece(square_from_algebraic('e', '1'), Color::White,
                  PieceType::King);
  board.clear();

  expect_empty_at(board, square_from_algebraic('e', '1'));
  EXPECT_EQ(board.all_occupancy(), 0U);
  EXPECT_EQ(board.occupancy(Color::White), 0U);
  EXPECT_EQ(board.occupancy(Color::Black), 0U);
  EXPECT_EQ(board.side_to_move(), Color::White);
  EXPECT_EQ(board.en_passant_square(), NO_SQUARE);
  EXPECT_EQ(board.halfmove_clock(), 0U);
  EXPECT_EQ(board.fullmove_number(), 1U);
  EXPECT_EQ(board.zobrist_key(), 0U);

  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, SetAndRemovePieceUpdatesOccupancy) {
  Board board;
  const Square e1 = square_from_algebraic('e', '1');
  const Square d1 = square_from_algebraic('d', '1');

  board.set_piece(e1, Color::White, PieceType::King);
  board.set_piece(d1, Color::White, PieceType::Queen);

  expect_piece_at(board, e1, Color::White, PieceType::King);
  expect_piece_at(board, d1, Color::White, PieceType::Queen);
  EXPECT_EQ(board.piece_on(e1), PieceType::King);
  EXPECT_EQ(board.color_on(d1), Color::White);
  EXPECT_NE(board.all_occupancy(), 0U);
  EXPECT_NE(board.occupancy(Color::White), 0U);
  EXPECT_EQ(board.occupancy(Color::Black), 0U);

  board.remove_piece(d1, Color::White, PieceType::Queen);
  expect_empty_at(board, d1);
  expect_piece_at(board, e1, Color::White, PieceType::King);

  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, FromFenLoadsFullRanksWithoutEmptyCounts) {
  // Each rank has eight explicit piece characters, so digit skipping is not
  // required. This validates the piece-placement portion of from_fen().
  const Board board = board_from_fen("bbbbbbbb/bbbbbbbb/bbbbbbbb/bbbbbbbb/"
                                     "bbbbbbbb/bbbbbbbb/bbbbbbbb/bbbbbbbb");

  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      expect_piece_at(board, square_from(file, rank), Color::Black,
                      PieceType::Bishop);
    }
  }

  log_board(board, std::clog);

  EXPECT_EQ(board.all_occupancy(), 0xFFFFFFFFFFFFFFFFULL);
  EXPECT_EQ(board.occupancy(Color::Black), 0xFFFFFFFFFFFFFFFFULL);
  EXPECT_EQ(board.occupancy(Color::White), 0U);

  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, FromFenLoadsMixedStartingBackRanks) {
  // First two ranks of the standard starting position have no empty-square
  // counts in the FEN string.
  const Board board = board_from_fen("rnbqkbnr/pppppppp");

  expect_piece_at(board, square_from_algebraic('a', '8'), Color::Black,
                  PieceType::Rook);
  expect_piece_at(board, square_from_algebraic('e', '8'), Color::Black,
                  PieceType::King);
  expect_piece_at(board, square_from_algebraic('a', '7'), Color::Black,
                  PieceType::Pawn);
  expect_piece_at(board, square_from_algebraic('h', '7'), Color::Black,
                  PieceType::Pawn);

  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, KingSquareFindsCorrectSquare) {
  Board board;
  const Square white_king = square_from_algebraic('e', '1');
  const Square black_king = square_from_algebraic('e', '8');

  board.set_piece(white_king, Color::White, PieceType::King);
  board.set_piece(black_king, Color::Black, PieceType::King);

  EXPECT_EQ(board.king_square(Color::White), white_king);
  EXPECT_EQ(board.king_square(Color::Black), black_king);

  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, RecomputeDerivedStateRebuildsOccupancyAndHash) {
  Board board;
  board.clear();

  board.set_piece(square_from_algebraic('a', '1'), Color::White,
                  PieceType::Rook);
  board.set_piece(square_from_algebraic('h', '8'), Color::Black,
                  PieceType::King);

  board.recompute_derived_state();
  const HashKey hash_after_first_recompute = board.zobrist_key();

  expect_piece_at(board, square_from_algebraic('a', '1'), Color::White,
                  PieceType::Rook);
  expect_piece_at(board, square_from_algebraic('h', '8'), Color::Black,
                  PieceType::King);
  EXPECT_NE(board.all_occupancy(), 0U);

  board.recompute_derived_state();
  EXPECT_EQ(board.zobrist_key(), hash_after_first_recompute);

  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, PieceBitboardsMatchSquareQueries) {
  Board board;
  board.set_piece(square_from_algebraic('c', '3'), Color::White,
                  PieceType::Knight);
  board.set_piece(square_from_algebraic('f', '6'), Color::Black,
                  PieceType::Bishop);

  const Bitboard white_knights = board.pieces(Color::White, PieceType::Knight);
  const Bitboard black_bishops = board.pieces(Color::Black, PieceType::Bishop);

  EXPECT_NE(white_knights & (1ULL << square_from_algebraic('c', '3')), 0U);
  EXPECT_NE(black_bishops & (1ULL << square_from_algebraic('f', '6')), 0U);
  EXPECT_EQ(white_knights & (1ULL << square_from_algebraic('f', '6')), 0U);

  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, LogBoardIncludesVisualBoardAndState) {
  Board board;
  board.set_piece(square_from_algebraic('e', '4'), Color::White,
                  PieceType::Pawn);
  board.set_piece(square_from_algebraic('e', '7'), Color::Black,
                  PieceType::Pawn);

  const std::string output = log_board_to_string(board);

  EXPECT_NE(output.find("Board"), std::string::npos);
  EXPECT_NE(output.find("Position:"), std::string::npos);
  EXPECT_NE(output.find("Side to move:"), std::string::npos);
  EXPECT_NE(output.find("Occupancy:"), std::string::npos);
  EXPECT_NE(output.find("Piece bitboards:"), std::string::npos);
  EXPECT_NE(output.find("P"), std::string::npos);
  EXPECT_NE(output.find("p"), std::string::npos);
}

TEST(BoardTest, StartingPositionMatchesExpectedPiecePlacement) {
  Board board = Board::starting_position();

  expect_piece_at(board, square_from_algebraic('e', '1'), Color::White,
                  PieceType::King);
  expect_piece_at(board, square_from_algebraic('d', '1'), Color::White,
                  PieceType::Queen);
  expect_piece_at(board, square_from_algebraic('a', '1'), Color::White,
                  PieceType::Rook);
  expect_piece_at(board, square_from_algebraic('e', '8'), Color::Black,
                  PieceType::King);
  expect_piece_at(board, square_from_algebraic('a', '7'), Color::Black,
                  PieceType::Pawn);
  expect_piece_at(board, square_from_algebraic('a', '2'), Color::White,
                  PieceType::Pawn);

  expect_empty_at(board, square_from_algebraic('e', '4'));
  expect_empty_at(board, square_from_algebraic('e', '5'));

  log_board(board, std::clog);

  SCOPED_TRACE(log_board_to_string(board));
}

} // namespace chesspp::core
