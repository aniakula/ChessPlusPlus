#include "board.hpp"
#include "game.hpp"
#include "types.hpp"

#include <gtest/gtest.h>

namespace chesspp::core {
namespace {

Square sq(char file, char rank) { return square_from_algebraic(file, rank); }

Game game_from_fen(const char *fen) { return Game(Board::from_fen(fen)); }

// ---------------------------------------------------------------------------
// Baseline: starting position
// ---------------------------------------------------------------------------

TEST(GameStateTest, StartingPositionIsQuietAndOngoing) {
  Game game;

  EXPECT_FALSE(game.is_check());
  EXPECT_FALSE(game.is_checkmate());
  EXPECT_FALSE(game.is_stalemate());
  EXPECT_FALSE(game.is_draw());
  EXPECT_EQ(game.draw_reason(), DrawReason::None);
  EXPECT_EQ(game.result(), GameResult::Ongoing);
  EXPECT_EQ(game.legal_moves().size(), 20U);
}

// ---------------------------------------------------------------------------
// is_check
// ---------------------------------------------------------------------------

TEST(GameStateTest, IsCheckTrueWhenSideToMoveKingAttacked) {
  const Game rook_check = game_from_fen("4k3/8/8/8/8/8/4r3/4K3 w - - 0 1");
  EXPECT_TRUE(rook_check.is_check());

  const Game knight_check = game_from_fen("4k3/8/8/8/8/5n2/8/4K3 w - - 0 1");
  EXPECT_TRUE(knight_check.is_check());
}

TEST(GameStateTest, IsCheckFalseWhenOnlyOpponentKingAttacked) {
  // White rook attacks the black king, but it is White to move, so the side to
  // move is not in check.
  const Game game = game_from_fen("4k3/4R3/8/8/8/8/8/4K3 w - - 0 1");
  EXPECT_FALSE(game.is_check());
}

// ---------------------------------------------------------------------------
// Checkmate
// ---------------------------------------------------------------------------

TEST(GameStateTest, BackRankCheckmateWhiteWins) {
  // Black is to move and is mated on the back rank, so White wins.
  const Game game = game_from_fen("R5k1/5ppp/8/8/8/8/8/6K1 b - - 0 1");

  EXPECT_TRUE(game.is_check());
  EXPECT_TRUE(game.is_checkmate());
  EXPECT_FALSE(game.is_stalemate());
  EXPECT_TRUE(game.legal_moves().empty());
  EXPECT_FALSE(game.is_draw());
  EXPECT_EQ(game.result(), GameResult::WhiteWins);
}

TEST(GameStateTest, FoolsMateBlackWins) {
  // White is to move and is mated, so Black wins.
  const Game game = game_from_fen(
      "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3");

  EXPECT_TRUE(game.is_check());
  EXPECT_TRUE(game.is_checkmate());
  EXPECT_TRUE(game.legal_moves().empty());
  EXPECT_EQ(game.result(), GameResult::BlackWins);
}

TEST(GameStateTest, CheckWithEscapeIsNotCheckmate) {
  const Game game = game_from_fen("4k3/8/8/8/8/8/4q3/4K3 w - - 0 1");

  EXPECT_TRUE(game.is_check());
  EXPECT_FALSE(game.is_checkmate());
  EXPECT_EQ(game.legal_moves().size(), 1U); // only Kxe2
  EXPECT_EQ(game.result(), GameResult::Ongoing);
}

// ---------------------------------------------------------------------------
// Stalemate
// ---------------------------------------------------------------------------

TEST(GameStateTest, StalemateIsDrawNotCheckmate) {
  const Game game = game_from_fen("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");

  EXPECT_FALSE(game.is_check());
  EXPECT_TRUE(game.is_stalemate());
  EXPECT_FALSE(game.is_checkmate());
  EXPECT_TRUE(game.legal_moves().empty());
  EXPECT_TRUE(game.is_draw());
  EXPECT_EQ(game.draw_reason(), DrawReason::Stalemate);
  EXPECT_EQ(game.result(), GameResult::Draw);
}

// ---------------------------------------------------------------------------
// Fifty-move rule (implemented threshold: halfmove clock == 50)
// ---------------------------------------------------------------------------

TEST(GameDrawTest, FiftyMoveRuleBoundary) {
  const Game just_before =
      game_from_fen("4k3/8/8/8/8/8/4R3/4K3 w - - 49 60");
  EXPECT_EQ(just_before.draw_reason(), DrawReason::None);
  EXPECT_FALSE(just_before.is_draw());

  const Game at_threshold =
      game_from_fen("4k3/8/8/8/8/8/4R3/4K3 w - - 50 60");
  EXPECT_EQ(at_threshold.draw_reason(), DrawReason::FiftyMoveRule);
  EXPECT_TRUE(at_threshold.is_draw());
  EXPECT_EQ(at_threshold.result(), GameResult::Draw);
}

// ---------------------------------------------------------------------------
// Insufficient material
// ---------------------------------------------------------------------------

TEST(GameDrawTest, InsufficientMaterialVariants) {
  const char *insufficient[] = {
      "4k3/8/8/8/8/8/8/4K3 w - - 0 1",   // K vs K
      "4k3/8/8/8/8/8/8/4KB2 w - - 0 1",  // K+B vs K
      "4k3/8/8/8/8/8/8/4KN2 w - - 0 1",  // K+N vs K
      "4kb2/8/8/8/8/8/8/4KB2 w - - 0 1", // K+B vs K+B
  };

  for (const char *fen : insufficient) {
    const Game game = game_from_fen(fen);
    EXPECT_EQ(game.draw_reason(), DrawReason::InsufficientMaterial)
        << "expected insufficient material for: " << fen;
    EXPECT_TRUE(game.is_draw()) << fen;
    EXPECT_EQ(game.result(), GameResult::Draw) << fen;
  }
}

TEST(GameDrawTest, SufficientMaterialIsNotInsufficientDraw) {
  const char *sufficient[] = {
      "4k3/8/8/8/8/8/8/4KR2 w - - 0 1",   // K+R vs K
      "4k3/8/8/8/8/8/4P3/4K3 w - - 0 1",  // K+P vs K
      "4k3/8/8/8/8/8/8/3QK3 w - - 0 1",   // K+Q vs K
      "4k3/8/8/8/8/8/8/4KBB1 w - - 0 1",  // K + two bishops
      "4k3/8/8/8/8/8/8/3BKN2 w - - 0 1",  // K + bishop + knight
  };

  for (const char *fen : sufficient) {
    const Game game = game_from_fen(fen);
    EXPECT_EQ(game.draw_reason(), DrawReason::None)
        << "did not expect a draw for: " << fen;
    EXPECT_FALSE(game.is_draw()) << fen;
    EXPECT_EQ(game.result(), GameResult::Ongoing) << fen;
  }
}

// ---------------------------------------------------------------------------
// Threefold repetition
// ---------------------------------------------------------------------------

TEST(GameDrawTest, ThreefoldRepetitionDetectedAfterThirdOccurrence) {
  // Rooks present so the position is never "insufficient material"; shuffling
  // the rooks back and forth reproduces the same position.
  Game game = game_from_fen("r3k3/8/8/8/8/8/8/R3K3 w - - 0 1");

  const Move w_out(sq('a', '1'), sq('b', '1'));
  const Move b_out(sq('a', '8'), sq('b', '8'));
  const Move w_back(sq('b', '1'), sq('a', '1'));
  const Move b_back(sq('b', '8'), sq('a', '8'));

  // First full cycle: position seen a second time.
  ASSERT_TRUE(game.try_make_move(w_out));
  ASSERT_TRUE(game.try_make_move(b_out));
  ASSERT_TRUE(game.try_make_move(w_back));
  ASSERT_TRUE(game.try_make_move(b_back));
  EXPECT_EQ(game.draw_reason(), DrawReason::None);

  // Second full cycle: position seen a third time -> threefold.
  ASSERT_TRUE(game.try_make_move(w_out));
  ASSERT_TRUE(game.try_make_move(b_out));
  ASSERT_TRUE(game.try_make_move(w_back));
  ASSERT_TRUE(game.try_make_move(b_back));

  EXPECT_EQ(game.draw_reason(), DrawReason::ThreefoldRepetition);
  EXPECT_TRUE(game.is_draw());
  EXPECT_EQ(game.result(), GameResult::Draw);
}

// ---------------------------------------------------------------------------
// is_legal / try_make_move / legal_moves
// ---------------------------------------------------------------------------

TEST(GameMoveTest, IsLegalRespectsExactFlags) {
  const Game game;

  EXPECT_TRUE(game.is_legal(
      Move(sq('e', '2'), sq('e', '4'), PieceType::None, MoveFlag::DoublePawnPush)));
  EXPECT_TRUE(game.is_legal(Move(sq('e', '2'), sq('e', '3'))));
  EXPECT_TRUE(game.is_legal(Move(sq('g', '1'), sq('f', '3'))));

  // Correct squares but wrong flags is not a legal move object.
  EXPECT_FALSE(game.is_legal(Move(sq('e', '2'), sq('e', '4'))));
  // Impossible move.
  EXPECT_FALSE(game.is_legal(Move(sq('e', '2'), sq('e', '5'))));
}

TEST(GameMoveTest, TryMakeMoveAppliesLegalMove) {
  Game game;

  ASSERT_TRUE(game.try_make_move(
      Move(sq('e', '2'), sq('e', '4'), PieceType::None, MoveFlag::DoublePawnPush)));

  const Board &board = game.board();
  EXPECT_EQ(board.side_to_move(), Color::Black);
  EXPECT_EQ(board.piece_and_color_on(sq('e', '4')),
            std::make_pair(Color::White, PieceType::Pawn));
  EXPECT_FALSE(board.piece_on(sq('e', '2')).has_value());
}

TEST(GameMoveTest, TryMakeMoveRejectsIllegalMoveAndLeavesStateUnchanged) {
  Game game;
  const HashKey key_before = game.board().zobrist_key();

  EXPECT_FALSE(game.try_make_move(Move(sq('e', '2'), sq('e', '5'))));

  EXPECT_EQ(game.board().side_to_move(), Color::White);
  EXPECT_EQ(game.board().zobrist_key(), key_before);
  EXPECT_EQ(game.board().piece_and_color_on(sq('e', '2')),
            std::make_pair(Color::White, PieceType::Pawn));
}

TEST(GameMoveTest, TryMakeMoveRejectsMoveThatLeavesKingInCheck) {
  // White rook on e3 is pinned by black rook on e8.
  Game game = game_from_fen("4r1k1/8/8/8/8/4R3/8/4K3 w - - 0 1");

  EXPECT_FALSE(game.try_make_move(Move(sq('e', '3'), sq('d', '3'))));
  EXPECT_EQ(game.board().side_to_move(), Color::White);

  EXPECT_TRUE(game.try_make_move(Move(sq('e', '3'), sq('e', '4'))));
  EXPECT_EQ(game.board().side_to_move(), Color::Black);
}

// ---------------------------------------------------------------------------
// reset / load_position
// ---------------------------------------------------------------------------

TEST(GameMoveTest, ResetRestoresStartingPosition) {
  Game game;
  ASSERT_TRUE(game.try_make_move(
      Move(sq('e', '2'), sq('e', '4'), PieceType::None, MoveFlag::DoublePawnPush)));
  ASSERT_TRUE(game.try_make_move(
      Move(sq('e', '7'), sq('e', '5'), PieceType::None, MoveFlag::DoublePawnPush)));

  game.reset();

  EXPECT_EQ(game.board().side_to_move(), Color::White);
  EXPECT_EQ(game.board().zobrist_key(),
            Board::starting_position().zobrist_key());
  EXPECT_EQ(game.legal_moves().size(), 20U);
  EXPECT_EQ(game.draw_reason(), DrawReason::None);
}

TEST(GameMoveTest, LoadPositionReplacesBoard) {
  Game game;
  game.load_position(Board::from_fen("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1"));

  EXPECT_EQ(game.board().side_to_move(), Color::Black);
  EXPECT_TRUE(game.is_stalemate());
  EXPECT_EQ(game.draw_reason(), DrawReason::Stalemate);
}

} // namespace
} // namespace chesspp::core
