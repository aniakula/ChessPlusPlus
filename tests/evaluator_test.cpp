#include "board.hpp"
#include "evaluator.hpp"
#include "types.hpp"

#include <gtest/gtest.h>

namespace chesspp::engine {
namespace {

core::Board play(std::initializer_list<core::Move> moves) {
  core::Board board = core::Board::starting_position();
  for (const core::Move move : moves) {
    core::UndoState undo{};
    board.make_move(move, undo);
  }
  return board;
}

core::Move quiet(char from_file, char from_rank, char to_file, char to_rank) {
  return core::Move(core::square_from_algebraic(from_file, from_rank),
                    core::square_from_algebraic(to_file, to_rank));
}

} // namespace

TEST(EvaluatorTest, StartingPositionIsEqual) {
  Evaluator eval;
  const core::Board board = core::Board::starting_position();
  EXPECT_EQ(eval.evaluate(board), 0);
  EXPECT_EQ(eval.positional_score(board, core::Color::White),
            eval.positional_score(board, core::Color::Black));
}

TEST(EvaluatorTest, AfterE4E5ScoreIsZero) {
  Evaluator eval;
  const core::Board board =
      play({quiet('e', '2', 'e', '4'), quiet('e', '7', 'e', '5')});
  // Open game is mirror-symmetric in the PSTs, so white-relative eval is 0.
  EXPECT_EQ(eval.evaluate(board), 0);
}

TEST(EvaluatorTest, AfterE4E6ScoreIsPlusFourteen) {
  Evaluator eval;
  const core::Board board =
      play({quiet('e', '2', 'e', '4'), quiet('e', '7', 'e', '6')});
  // e4 gains +32 PST; e6 only gains +18, so white is +0.14 pawns.
  EXPECT_EQ(eval.evaluate(board), 14);
}

} // namespace chesspp::engine
