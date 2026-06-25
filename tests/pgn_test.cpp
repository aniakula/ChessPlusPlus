#include "board.hpp"
#include "pgn.hpp"
#include "types.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace chesspp::core {
namespace {

void expect_move(const Move &move, Square from, Square to,
                 PieceType promotion = PieceType::None,
                 MoveFlag flags = MoveFlag::Quiet) {
  EXPECT_EQ(move.from(), from);
  EXPECT_EQ(move.to(), to);
  EXPECT_EQ(move.promotion(), promotion);
  EXPECT_EQ(move.flags(), flags);
}

TEST(PgnTest, ParsesBasicSanFromStartingPosition) {
  const std::vector<Move> moves =
      parse_pgn_moves("1. e4 e5 2. Nf3 Nc6 3. Bb5 a6");

  ASSERT_EQ(moves.size(), 6U);
  expect_move(moves[0], square_from_algebraic('e', '2'),
              square_from_algebraic('e', '4'), PieceType::None,
              MoveFlag::DoublePawnPush);
  expect_move(moves[1], square_from_algebraic('e', '7'),
              square_from_algebraic('e', '5'), PieceType::None,
              MoveFlag::DoublePawnPush);
  expect_move(moves[2], square_from_algebraic('g', '1'),
              square_from_algebraic('f', '3'));
  expect_move(moves[3], square_from_algebraic('b', '8'),
              square_from_algebraic('c', '6'));
  expect_move(moves[4], square_from_algebraic('f', '1'),
              square_from_algebraic('b', '5'));
  expect_move(moves[5], square_from_algebraic('a', '7'),
              square_from_algebraic('a', '6'));
}

TEST(PgnTest, IgnoresHeadersCommentsVariationsAnnotationsAndResult) {
  const std::string pgn = "[Event \"Example\"]\n"
                          "[Site \"?\"]\n\n"
                          "1. e4! {best by test} e5 (1... c5) 2. Nf3?! Nc6 1-0";

  const std::vector<Move> moves = parse_pgn_moves(pgn);

  ASSERT_EQ(moves.size(), 4U);
  expect_move(moves[0], square_from_algebraic('e', '2'),
              square_from_algebraic('e', '4'), PieceType::None,
              MoveFlag::DoublePawnPush);
  expect_move(moves[1], square_from_algebraic('e', '7'),
              square_from_algebraic('e', '5'), PieceType::None,
              MoveFlag::DoublePawnPush);
  expect_move(moves[2], square_from_algebraic('g', '1'),
              square_from_algebraic('f', '3'));
  expect_move(moves[3], square_from_algebraic('b', '8'),
              square_from_algebraic('c', '6'));
}

TEST(PgnTest, ParsesCastling) {
  const std::vector<Move> moves =
      parse_pgn_moves("1. e4 e5 2. Nf3 Nc6 3. Bc4 Bc5 4. O-O Nf6");

  ASSERT_EQ(moves.size(), 8U);
  expect_move(moves[6], square_from_algebraic('e', '1'),
              square_from_algebraic('g', '1'), PieceType::None,
              MoveFlag::KingCastle);
}

TEST(PgnTest, ParsesEnPassantCapture) {
  const std::vector<Move> moves = parse_pgn_moves("1. e4 a6 2. e5 d5 3. exd6");

  ASSERT_EQ(moves.size(), 5U);
  expect_move(moves[4], square_from_algebraic('e', '5'),
              square_from_algebraic('d', '6'), PieceType::None,
              MoveFlag::Capture | MoveFlag::EnPassant);
}

TEST(PgnTest, ParsesPromotionFromProvidedInitialPosition) {
  Board board = Board::from_fen("4k3/P7/8/8/8/8/8/4K3 w - - 0 1");

  const std::vector<Move> moves = parse_pgn_moves("1. a8=Q+", board);

  ASSERT_EQ(moves.size(), 1U);
  expect_move(moves[0], square_from_algebraic('a', '7'),
              square_from_algebraic('a', '8'), PieceType::Queen,
              MoveFlag::Promotion);
}

TEST(PgnTest, ParsesDisambiguatedKnightMove) {
  Board board = Board::from_fen("4k3/8/8/8/N7/3N4/8/4K3 w - - 0 1");

  const std::vector<Move> moves = parse_pgn_moves("1. Ndb2", board);

  ASSERT_EQ(moves.size(), 1U);
  expect_move(moves[0], square_from_algebraic('d', '3'),
              square_from_algebraic('b', '2'));
}

} // namespace
} // namespace chesspp::core
