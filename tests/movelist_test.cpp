#include "movegen.hpp"
#include "types.hpp"

#include <gtest/gtest.h>

#include <vector>

namespace chesspp::core {
namespace {

Square sq(char file, char rank) { return square_from_algebraic(file, rank); }

TEST(MoveListTest, DefaultConstructedIsEmpty) {
  MoveList list;

  EXPECT_EQ(list.size(), 0U);
  EXPECT_TRUE(list.empty());
  EXPECT_EQ(list.begin(), list.end());
}

TEST(MoveListTest, PushIncrementsSizeAndStoresInOrder) {
  MoveList list;
  const Move first(sq('e', '2'), sq('e', '4'), PieceType::None,
                   MoveFlag::DoublePawnPush);
  const Move second(sq('g', '1'), sq('f', '3'));
  const Move third(sq('a', '7'), sq('a', '8'), PieceType::Queen,
                   MoveFlag::Promotion);

  list.push(first);
  list.push(second);
  list.push(third);

  ASSERT_EQ(list.size(), 3U);
  EXPECT_FALSE(list.empty());
  EXPECT_EQ(list[0], first);
  EXPECT_EQ(list[1], second);
  EXPECT_EQ(list[2], third);
}

TEST(MoveListTest, IterationYieldsEveryMoveInPushOrder) {
  MoveList list;
  const std::vector<Move> expected = {
      Move(sq('b', '1'), sq('c', '3')),
      Move(sq('e', '2'), sq('e', '4'), PieceType::None,
           MoveFlag::DoublePawnPush),
      Move(sq('e', '1'), sq('g', '1'), PieceType::None, MoveFlag::KingCastle),
  };

  for (const Move &move : expected) {
    list.push(move);
  }

  std::vector<Move> collected;
  for (const Move &move : list) {
    collected.push_back(move);
  }

  ASSERT_EQ(collected.size(), expected.size());
  for (std::size_t index = 0; index < expected.size(); ++index) {
    EXPECT_EQ(collected[index], expected[index]) << "mismatch at " << index;
  }
}

TEST(MoveListTest, ClearResetsSizeButKeepsContainerUsable) {
  MoveList list;
  list.push(Move(sq('e', '2'), sq('e', '4'), PieceType::None,
                 MoveFlag::DoublePawnPush));
  list.push(Move(sq('d', '2'), sq('d', '4'), PieceType::None,
                 MoveFlag::DoublePawnPush));

  list.clear();

  EXPECT_EQ(list.size(), 0U);
  EXPECT_TRUE(list.empty());
  EXPECT_EQ(list.begin(), list.end());

  const Move reused(sq('g', '1'), sq('f', '3'));
  list.push(reused);
  ASSERT_EQ(list.size(), 1U);
  EXPECT_EQ(list[0], reused);
}

TEST(MoveListTest, ContainsMatchesIdenticalMove) {
  MoveList list;
  const Move move(sq('e', '2'), sq('e', '4'), PieceType::None,
                  MoveFlag::DoublePawnPush);
  list.push(move);

  EXPECT_TRUE(list.contains(move));
  EXPECT_TRUE(list.contains(Move(sq('e', '2'), sq('e', '4'), PieceType::None,
                                 MoveFlag::DoublePawnPush)));
}

TEST(MoveListTest, ContainsDistinguishesFlags) {
  MoveList list;
  list.push(Move(sq('e', '2'), sq('e', '4'), PieceType::None,
                 MoveFlag::DoublePawnPush));

  // Same squares, different flags must not be considered contained.
  EXPECT_FALSE(list.contains(Move(sq('e', '2'), sq('e', '4'), PieceType::None,
                                  MoveFlag::Quiet)));
}

TEST(MoveListTest, ContainsDistinguishesSquaresAndPromotionPiece) {
  MoveList list;
  list.push(Move(sq('a', '7'), sq('a', '8'), PieceType::Queen,
                 MoveFlag::Promotion));

  // Different destination.
  EXPECT_FALSE(list.contains(Move(sq('a', '7'), sq('b', '8'), PieceType::Queen,
                                  MoveFlag::Promotion)));
  // Different promotion piece.
  EXPECT_FALSE(list.contains(Move(sq('a', '7'), sq('a', '8'), PieceType::Knight,
                                  MoveFlag::Promotion)));
  // Exact match.
  EXPECT_TRUE(list.contains(Move(sq('a', '7'), sq('a', '8'), PieceType::Queen,
                                 MoveFlag::Promotion)));
}

TEST(MoveListTest, ContainsReturnsFalseOnEmptyList) {
  MoveList list;
  EXPECT_FALSE(list.contains(Move(sq('e', '2'), sq('e', '4'))));
}

} // namespace
} // namespace chesspp::core
