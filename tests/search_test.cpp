#include "search.hpp"

#include <gtest/gtest.h>

#include <sstream>

namespace chesspp::engine {
namespace {

TEST(SearchTest, SearchStatsStreamOutputIsHumanReadable) {
  SearchStats stats{};
  stats.nodes = 123;
  stats.transposition_hits = 4;
  stats.beta_cutoffs = 5;

  std::ostringstream out;
  out << stats;

  EXPECT_EQ(out.str(),
            "SearchStats{nodes=123, transposition_hits=4, beta_cutoffs=5}");
}

TEST(SearchTest, ReturnsALegalMoveFromStartingPosition) {
  Search search;
  core::Board board = core::Board::starting_position();
  const core::HashKey original_key = board.zobrist_key();
  core::MoveList legal_moves;
  core::MoveGenerator::generate_legal(board, legal_moves);

  const SearchResult result = search.search(board, SearchLimits{2, 0, 0});

  EXPECT_TRUE(legal_moves.contains(result.best_move));
  EXPECT_EQ(result.depth_reached, 2);
  EXPECT_GT(result.stats.nodes, 0U);
  EXPECT_EQ(board.zobrist_key(), original_key);
}

TEST(SearchTest, FindsMateInOne) {
  Search search;
  core::Board board =
      core::Board::from_fen("7k/8/5KQ1/8/8/8/8/8 w - - 0 1");

  const SearchResult result = search.search(board, SearchLimits{1, 0, 0});

  ASSERT_FALSE(result.best_move.is_null());
  core::UndoState undo{};
  board.make_move(result.best_move, undo);

  core::MoveList replies;
  core::MoveGenerator::generate_legal(board, replies);
  EXPECT_TRUE(replies.empty());
  EXPECT_TRUE(board.in_check(board.side_to_move()));
  EXPECT_EQ(result.score, core::MATE_SCORE - 1);
}

TEST(SearchTest, KeepsLegalFallbackWhenNodeLimitStopsFirstIteration) {
  Search search;
  core::Board board = core::Board::starting_position();
  core::MoveList legal_moves;
  core::MoveGenerator::generate_legal(board, legal_moves);

  const SearchResult result = search.search(board, SearchLimits{5, 1, 0});

  EXPECT_TRUE(legal_moves.contains(result.best_move));
  EXPECT_EQ(result.depth_reached, 0);
  EXPECT_EQ(result.stats.nodes, 1U);
}

} // namespace
} // namespace chesspp::engine
