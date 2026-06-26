#include "board.hpp"
#include "pgn.hpp"
#include "types.hpp"

#include <gtest/gtest.h>

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>

namespace chesspp::core {

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

void expect_board_state(const Board &board, Color side_to_move,
                        Square en_passant_square, std::uint16_t halfmove_clock,
                        std::uint16_t fullmove_number,
                        const CastlingRights &castling_rights,
                        HashKey zobrist_key) {
  EXPECT_EQ(board.side_to_move(), side_to_move);
  EXPECT_EQ(board.en_passant_square(), en_passant_square);
  EXPECT_EQ(board.halfmove_clock(), halfmove_clock);
  EXPECT_EQ(board.fullmove_number(), fullmove_number);
  EXPECT_EQ(board.castling_rights().white_kingside,
            castling_rights.white_kingside);
  EXPECT_EQ(board.castling_rights().white_queenside,
            castling_rights.white_queenside);
  EXPECT_EQ(board.castling_rights().black_kingside,
            castling_rights.black_kingside);
  EXPECT_EQ(board.castling_rights().black_queenside,
            castling_rights.black_queenside);
  EXPECT_EQ(board.zobrist_key(), zobrist_key);
}

void expect_boards_equivalent(const Board &lhs, const Board &rhs) {
  expect_board_state(lhs, rhs.side_to_move(), rhs.en_passant_square(),
                     rhs.halfmove_clock(), rhs.fullmove_number(),
                     rhs.castling_rights(), rhs.zobrist_key());
  EXPECT_EQ(lhs.all_occupancy(), rhs.all_occupancy());
  EXPECT_EQ(lhs.occupancy(Color::White), rhs.occupancy(Color::White));
  EXPECT_EQ(lhs.occupancy(Color::Black), rhs.occupancy(Color::Black));
}

// Full structural comparison: every piece bitboard, occupancy, all scalar
// state, castling rights, and (optionally) the zobrist key.
void expect_positions_identical(const Board &actual, const Board &expected,
                                bool check_hash = true) {
  for (Color color : {Color::White, Color::Black}) {
    for (PieceType piece :
         {PieceType::Pawn, PieceType::Knight, PieceType::Bishop,
          PieceType::Rook, PieceType::Queen, PieceType::King}) {
      EXPECT_EQ(actual.pieces(color, piece), expected.pieces(color, piece))
          << "Piece bitboard mismatch for color " << static_cast<int>(color)
          << " piece " << static_cast<int>(piece);
    }
  }

  EXPECT_EQ(actual.all_occupancy(), expected.all_occupancy());
  EXPECT_EQ(actual.occupancy(Color::White), expected.occupancy(Color::White));
  EXPECT_EQ(actual.occupancy(Color::Black), expected.occupancy(Color::Black));
  EXPECT_EQ(actual.side_to_move(), expected.side_to_move());
  EXPECT_EQ(actual.en_passant_square(), expected.en_passant_square());
  EXPECT_EQ(actual.halfmove_clock(), expected.halfmove_clock());
  EXPECT_EQ(actual.fullmove_number(), expected.fullmove_number());
  EXPECT_EQ(actual.castling_rights().white_kingside,
            expected.castling_rights().white_kingside);
  EXPECT_EQ(actual.castling_rights().white_queenside,
            expected.castling_rights().white_queenside);
  EXPECT_EQ(actual.castling_rights().black_kingside,
            expected.castling_rights().black_kingside);
  EXPECT_EQ(actual.castling_rights().black_queenside,
            expected.castling_rights().black_queenside);

  if (check_hash) {
    EXPECT_EQ(actual.zobrist_key(), expected.zobrist_key());
  }
}

namespace {
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

  log_board(board, std::clog, true);

  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, MoveStreamOutputIsHumanReadable) {
  std::ostringstream out;
  out << Move(square_from_algebraic('e', '7'), square_from_algebraic('e', '8'),
              PieceType::Queen, MoveFlag::Promotion | MoveFlag::Capture);

  EXPECT_EQ(out.str(), "e7e8=Q (capture, promotion)");
}

TEST(BoardTest, FromFenParsesTrailingStateFields) {
  const Board board = board_from_fen(
      "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2");

  EXPECT_EQ(board.side_to_move(), Color::Black);

  const CastlingRights rights = board.castling_rights();
  EXPECT_TRUE(rights.white_kingside);
  EXPECT_TRUE(rights.white_queenside);
  EXPECT_TRUE(rights.black_kingside);
  EXPECT_TRUE(rights.black_queenside);

  EXPECT_EQ(board.en_passant_square(), NO_SQUARE);
  EXPECT_EQ(board.halfmove_clock(), 1U);
  EXPECT_EQ(board.fullmove_number(), 2U);

  log_board(board, std::clog, true);

  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, FromFenParsesTrailingStateFieldsPartialCastlingRights) {
  const Board board = board_from_fen(
      "2r2rk1/pp1b1ppp/1q2pn2/2bpN3/3N1P2/3PP3/PP1B2PP/R2QKB1R w KQ - 3 13");

  EXPECT_EQ(board.side_to_move(), Color::White);

  const CastlingRights rights = board.castling_rights();
  EXPECT_TRUE(rights.white_kingside);
  EXPECT_TRUE(rights.white_queenside);
  EXPECT_FALSE(rights.black_kingside);
  EXPECT_FALSE(rights.black_queenside);

  EXPECT_EQ(board.en_passant_square(), NO_SQUARE);
  EXPECT_EQ(board.halfmove_clock(), 3U);
  EXPECT_EQ(board.fullmove_number(), 13U);

  log_board(board, std::clog, true);

  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, FromFenParsesTrailingStateFieldsWithEnPassant) {
  const Board board = board_from_fen(
      "rnbqkbnr/ppp1p1pp/3p4/4pp2/3PP3/5N2/PPP2PPP/RNBQKB1R w KQkq f6 0 4");

  EXPECT_EQ(board.side_to_move(), Color::White);

  const CastlingRights rights = board.castling_rights();
  EXPECT_TRUE(rights.white_kingside);
  EXPECT_TRUE(rights.white_queenside);
  EXPECT_TRUE(rights.black_kingside);
  EXPECT_TRUE(rights.black_queenside);

  EXPECT_EQ(board.en_passant_square(), square_from_algebraic('f', '6'));
  EXPECT_EQ(board.halfmove_clock(), 0U);
  EXPECT_EQ(board.fullmove_number(), 4U);

  log_board(board, std::clog, true);

  SCOPED_TRACE(log_board_to_string(board));
}

// ---------------------------------------------------------------------------
// Captures
// ---------------------------------------------------------------------------

TEST(BoardTest, MakeUnmakeWhitePawnCapture) {
  const char *start =
      "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2";
  const char *result =
      "rnbqkbnr/ppp1pppp/8/3P4/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2";

  Board board = board_from_fen(start);
  const Board snapshot = board;

  UndoState undo;
  const Move move(square_from_algebraic('e', '4'),
                  square_from_algebraic('d', '5'), PieceType::None,
                  MoveFlag::Capture);
  board.make_move(move, undo);

  expect_positions_identical(board, board_from_fen(result));
  expect_piece_at(board, square_from_algebraic('d', '5'), Color::White,
                  PieceType::Pawn);

  board.unmake_move(undo);
  expect_positions_identical(board, snapshot);
  expect_piece_at(board, square_from_algebraic('d', '5'), Color::Black,
                  PieceType::Pawn);
  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, MakeUnmakeBlackPawnCapture) {
  const char *start =
      "rnbqkbnr/pppp1ppp/8/4p3/3P4/8/PPP1PPPP/RNBQKBNR b KQkq - 0 2";
  const char *result =
      "rnbqkbnr/pppp1ppp/8/8/3p4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 3";

  Board board = board_from_fen(start);
  const Board snapshot = board;

  UndoState undo;
  const Move move(square_from_algebraic('e', '5'),
                  square_from_algebraic('d', '4'), PieceType::None,
                  MoveFlag::Capture);
  board.make_move(move, undo);

  expect_positions_identical(board, board_from_fen(result));

  board.unmake_move(undo);
  expect_positions_identical(board, snapshot);
  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, MakeUnmakePieceCaptureRestoresCapturedPiece) {
  // Knight captures a knight: validates non-pawn captured-piece restoration and
  // the halfmove-clock reset on a capture.
  const char *start = "4k3/8/8/4n3/2N5/8/8/4K3 w - - 5 20";
  const char *result = "4k3/8/8/4N3/8/8/8/4K3 b - - 0 20";

  Board board = board_from_fen(start);
  const Board snapshot = board;

  UndoState undo;
  const Move move(square_from_algebraic('c', '4'),
                  square_from_algebraic('e', '5'), PieceType::None,
                  MoveFlag::Capture);
  board.make_move(move, undo);

  expect_positions_identical(board, board_from_fen(result));
  EXPECT_EQ(board.halfmove_clock(), 0U);

  board.unmake_move(undo);
  expect_positions_identical(board, snapshot);
  expect_piece_at(board, square_from_algebraic('e', '5'), Color::Black,
                  PieceType::Knight);
  SCOPED_TRACE(log_board_to_string(board));
}

// ---------------------------------------------------------------------------
// En passant
// ---------------------------------------------------------------------------

TEST(BoardTest, MakeUnmakeWhiteEnPassant) {
  const char *start =
      "rnbqkbnr/ppp2ppp/4p3/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3";
  const char *result =
      "rnbqkbnr/ppp2ppp/3Pp3/8/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 3";

  Board board = board_from_fen(start);
  const Board snapshot = board;

  UndoState undo;
  const Move move(square_from_algebraic('e', '5'),
                  square_from_algebraic('d', '6'), PieceType::None,
                  MoveFlag::EnPassant | MoveFlag::Capture);
  board.make_move(move, undo);

  expect_positions_identical(board, board_from_fen(result));
  expect_piece_at(board, square_from_algebraic('d', '6'), Color::White,
                  PieceType::Pawn);
  expect_empty_at(board, square_from_algebraic('d', '5'));

  board.unmake_move(undo);
  expect_positions_identical(board, snapshot);
  expect_piece_at(board, square_from_algebraic('d', '5'), Color::Black,
                  PieceType::Pawn);
  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, MakeUnmakeBlackEnPassant) {
  const char *start =
      "rnbqkbnr/ppp1pppp/8/8/3pP3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 3";
  const char *result =
      "rnbqkbnr/ppp1pppp/8/8/8/4p3/PPPP1PPP/RNBQKBNR w KQkq - 0 4";

  Board board = board_from_fen(start);
  const Board snapshot = board;

  UndoState undo;
  const Move move(square_from_algebraic('d', '4'),
                  square_from_algebraic('e', '3'), PieceType::None,
                  MoveFlag::EnPassant | MoveFlag::Capture);
  board.make_move(move, undo);

  expect_positions_identical(board, board_from_fen(result));
  expect_piece_at(board, square_from_algebraic('e', '3'), Color::Black,
                  PieceType::Pawn);
  expect_empty_at(board, square_from_algebraic('e', '4'));

  board.unmake_move(undo);
  expect_positions_identical(board, snapshot);
  expect_piece_at(board, square_from_algebraic('e', '4'), Color::White,
                  PieceType::Pawn);
  SCOPED_TRACE(log_board_to_string(board));
}

// ---------------------------------------------------------------------------
// Castling: pieces and game state (hash checked separately below)
// ---------------------------------------------------------------------------

TEST(BoardTest, MakeUnmakeWhiteKingsideCastle) {
  const char *start = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1";
  const char *result = "r3k2r/8/8/8/8/8/8/R4RK1 b kq - 1 1";

  Board board = board_from_fen(start);
  const Board snapshot = board;

  UndoState undo;
  const Move move(square_from_algebraic('e', '1'),
                  square_from_algebraic('g', '1'), PieceType::None,
                  MoveFlag::KingCastle);
  board.make_move(move, undo);

  expect_positions_identical(board, board_from_fen(result),
                             /*check_hash=*/false);
  expect_piece_at(board, square_from_algebraic('g', '1'), Color::White,
                  PieceType::King);
  expect_piece_at(board, square_from_algebraic('f', '1'), Color::White,
                  PieceType::Rook);

  board.unmake_move(undo);
  expect_positions_identical(board, snapshot);
  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, MakeUnmakeWhiteQueensideCastle) {
  const char *start = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1";
  const char *result = "r3k2r/8/8/8/8/8/8/2KR3R b kq - 1 1";

  Board board = board_from_fen(start);
  const Board snapshot = board;

  UndoState undo;
  const Move move(square_from_algebraic('e', '1'),
                  square_from_algebraic('c', '1'), PieceType::None,
                  MoveFlag::QueenCastle);
  board.make_move(move, undo);

  expect_positions_identical(board, board_from_fen(result),
                             /*check_hash=*/false);
  expect_piece_at(board, square_from_algebraic('c', '1'), Color::White,
                  PieceType::King);
  expect_piece_at(board, square_from_algebraic('d', '1'), Color::White,
                  PieceType::Rook);

  board.unmake_move(undo);
  expect_positions_identical(board, snapshot);
  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, MakeUnmakeBlackKingsideCastle) {
  const char *start = "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1";
  const char *result = "r4rk1/8/8/8/8/8/8/R3K2R w KQ - 1 2";

  Board board = board_from_fen(start);
  const Board snapshot = board;

  UndoState undo;
  const Move move(square_from_algebraic('e', '8'),
                  square_from_algebraic('g', '8'), PieceType::None,
                  MoveFlag::KingCastle);
  board.make_move(move, undo);

  expect_positions_identical(board, board_from_fen(result),
                             /*check_hash=*/false);
  expect_piece_at(board, square_from_algebraic('g', '8'), Color::Black,
                  PieceType::King);
  expect_piece_at(board, square_from_algebraic('f', '8'), Color::Black,
                  PieceType::Rook);

  board.unmake_move(undo);
  expect_positions_identical(board, snapshot);
  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, MakeUnmakeBlackQueensideCastle) {
  const char *start = "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1";
  const char *result = "2kr3r/8/8/8/8/8/8/R3K2R w KQ - 1 2";

  Board board = board_from_fen(start);
  const Board snapshot = board;

  UndoState undo;
  const Move move(square_from_algebraic('e', '8'),
                  square_from_algebraic('c', '8'), PieceType::None,
                  MoveFlag::QueenCastle);
  board.make_move(move, undo);

  expect_positions_identical(board, board_from_fen(result),
                             /*check_hash=*/false);
  expect_piece_at(board, square_from_algebraic('c', '8'), Color::Black,
                  PieceType::King);
  expect_piece_at(board, square_from_algebraic('d', '8'), Color::Black,
                  PieceType::Rook);

  board.unmake_move(undo);
  expect_positions_identical(board, snapshot);
  SCOPED_TRACE(log_board_to_string(board));
}

// Hash correctness for castling: an incrementally-updated key after castling
// must equal the key produced by loading the same resulting position fresh.
TEST(BoardTest, CastlingHashMatchesFreshlyLoadedPosition) {
  Board board = board_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");

  UndoState undo;
  const Move move(square_from_algebraic('e', '1'),
                  square_from_algebraic('g', '1'), PieceType::None,
                  MoveFlag::KingCastle);
  board.make_move(move, undo);

  const Board expected = board_from_fen("r3k2r/8/8/8/8/8/8/R4RK1 b kq - 1 1");
  EXPECT_EQ(board.zobrist_key(), expected.zobrist_key());
  SCOPED_TRACE(log_board_to_string(board));
}

// ---------------------------------------------------------------------------
// Promotion
// ---------------------------------------------------------------------------

TEST(BoardTest, MakeUnmakeWhitePromotionQuiet) {
  const char *start = "4k3/P7/8/8/8/8/8/4K3 w - - 0 1";
  const char *result = "Q3k3/8/8/8/8/8/8/4K3 b - - 0 1";

  Board board = board_from_fen(start);
  const Board snapshot = board;

  UndoState undo;
  const Move move(square_from_algebraic('a', '7'),
                  square_from_algebraic('a', '8'), PieceType::Queen,
                  MoveFlag::Promotion);
  board.make_move(move, undo);

  expect_positions_identical(board, board_from_fen(result));
  expect_piece_at(board, square_from_algebraic('a', '8'), Color::White,
                  PieceType::Queen);

  board.unmake_move(undo);
  expect_positions_identical(board, snapshot);
  expect_piece_at(board, square_from_algebraic('a', '7'), Color::White,
                  PieceType::Pawn);
  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, MakeUnmakeWhitePromotionCapture) {
  // Pawn b7 captures a knight on a8 and promotes to a queen.
  const char *start = "n3k3/1P6/8/8/8/8/8/4K3 w - - 0 1";
  const char *result = "Q3k3/8/8/8/8/8/8/4K3 b - - 0 1";

  Board board = board_from_fen(start);
  const Board snapshot = board;

  UndoState undo;
  const Move move(square_from_algebraic('b', '7'),
                  square_from_algebraic('a', '8'), PieceType::Queen,
                  MoveFlag::Promotion | MoveFlag::Capture);
  board.make_move(move, undo);

  expect_positions_identical(board, board_from_fen(result));
  expect_piece_at(board, square_from_algebraic('a', '8'), Color::White,
                  PieceType::Queen);

  board.unmake_move(undo);
  expect_positions_identical(board, snapshot);
  expect_piece_at(board, square_from_algebraic('a', '8'), Color::Black,
                  PieceType::Knight);
  expect_piece_at(board, square_from_algebraic('b', '7'), Color::White,
                  PieceType::Pawn);
  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, MakeUnmakeBlackPromotionQuiet) {
  const char *start = "4k3/8/8/8/8/8/p7/4K3 b - - 0 1";
  const char *result = "4k3/8/8/8/8/8/8/q3K3 w - - 0 2";

  Board board = board_from_fen(start);
  const Board snapshot = board;

  UndoState undo;
  const Move move(square_from_algebraic('a', '2'),
                  square_from_algebraic('a', '1'), PieceType::Queen,
                  MoveFlag::Promotion);
  board.make_move(move, undo);

  expect_positions_identical(board, board_from_fen(result));
  expect_piece_at(board, square_from_algebraic('a', '1'), Color::Black,
                  PieceType::Queen);

  board.unmake_move(undo);
  expect_positions_identical(board, snapshot);
  expect_piece_at(board, square_from_algebraic('a', '2'), Color::Black,
                  PieceType::Pawn);
  SCOPED_TRACE(log_board_to_string(board));
}

// ---------------------------------------------------------------------------
// Double pawn push
// ---------------------------------------------------------------------------

TEST(BoardTest, MakeUnmakeWhiteDoublePawnPush) {
  const char *result =
      "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";

  Board board = Board::starting_position();
  const Board snapshot = board;

  UndoState undo;
  const Move move(square_from_algebraic('e', '2'),
                  square_from_algebraic('e', '4'), PieceType::None,
                  MoveFlag::DoublePawnPush);
  board.make_move(move, undo);

  expect_positions_identical(board, board_from_fen(result));
  EXPECT_EQ(board.en_passant_square(), square_from_algebraic('e', '3'));

  board.unmake_move(undo);
  expect_positions_identical(board, snapshot);
  EXPECT_EQ(board.en_passant_square(), NO_SQUARE);
  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, MakeUnmakeBlackDoublePawnPush) {
  const char *start =
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1";
  const char *result =
      "rnbqkbnr/pppp1ppp/8/4p3/8/8/PPPPPPPP/RNBQKBNR w KQkq e6 0 2";

  Board board = board_from_fen(start);
  const Board snapshot = board;

  UndoState undo;
  const Move move(square_from_algebraic('e', '7'),
                  square_from_algebraic('e', '5'), PieceType::None,
                  MoveFlag::DoublePawnPush);
  board.make_move(move, undo);

  expect_positions_identical(board, board_from_fen(result));
  EXPECT_EQ(board.en_passant_square(), square_from_algebraic('e', '6'));

  board.unmake_move(undo);
  expect_positions_identical(board, snapshot);
  SCOPED_TRACE(log_board_to_string(board));
}

// ---------------------------------------------------------------------------
// Castling-rights updates from king / rook moves
// ---------------------------------------------------------------------------

TEST(BoardTest, WhiteKingMoveClearsBothCastlingRights) {
  Board board = board_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");

  UndoState undo;
  const Move move(square_from_algebraic('e', '1'),
                  square_from_algebraic('e', '2'), PieceType::None,
                  MoveFlag::Quiet);
  board.make_move(move, undo);

  expect_positions_identical(
      board, board_from_fen("r3k2r/8/8/8/8/8/4K3/R6R b kq - 1 1"));
  const CastlingRights rights = board.castling_rights();
  EXPECT_FALSE(rights.white_kingside);
  EXPECT_FALSE(rights.white_queenside);
  EXPECT_TRUE(rights.black_kingside);
  EXPECT_TRUE(rights.black_queenside);
  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, BlackKingMoveClearsBothCastlingRights) {
  Board board = board_from_fen("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1");

  UndoState undo;
  const Move move(square_from_algebraic('e', '8'),
                  square_from_algebraic('e', '7'), PieceType::None,
                  MoveFlag::Quiet);
  board.make_move(move, undo);

  expect_positions_identical(
      board, board_from_fen("r6r/4k3/8/8/8/8/8/R3K2R w KQ - 1 2"));
  const CastlingRights rights = board.castling_rights();
  EXPECT_TRUE(rights.white_kingside);
  EXPECT_TRUE(rights.white_queenside);
  EXPECT_FALSE(rights.black_kingside);
  EXPECT_FALSE(rights.black_queenside);
  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, WhiteQueenRookMoveClearsQueensideRight) {
  Board board = board_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");

  UndoState undo;
  const Move move(square_from_algebraic('a', '1'),
                  square_from_algebraic('d', '1'), PieceType::None,
                  MoveFlag::Quiet);
  board.make_move(move, undo);

  expect_positions_identical(
      board, board_from_fen("r3k2r/8/8/8/8/8/8/3RK2R b Kkq - 1 1"));
  const CastlingRights rights = board.castling_rights();
  EXPECT_TRUE(rights.white_kingside);
  EXPECT_FALSE(rights.white_queenside);
  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, WhiteKingRookMoveClearsKingsideRight) {
  Board board = board_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");

  UndoState undo;
  const Move move(square_from_algebraic('h', '1'),
                  square_from_algebraic('g', '1'), PieceType::None,
                  MoveFlag::Quiet);
  board.make_move(move, undo);

  expect_positions_identical(
      board, board_from_fen("r3k2r/8/8/8/8/8/8/R3K1R1 b Qkq - 1 1"));
  const CastlingRights rights = board.castling_rights();
  EXPECT_FALSE(rights.white_kingside);
  EXPECT_TRUE(rights.white_queenside);
  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, BlackQueenRookMoveClearsQueensideRight) {
  Board board = board_from_fen("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1");

  UndoState undo;
  const Move move(square_from_algebraic('a', '8'),
                  square_from_algebraic('d', '8'), PieceType::None,
                  MoveFlag::Quiet);
  board.make_move(move, undo);

  expect_positions_identical(
      board, board_from_fen("3rk2r/8/8/8/8/8/8/R3K2R w KQk - 1 2"));
  const CastlingRights rights = board.castling_rights();
  EXPECT_TRUE(rights.black_kingside);
  EXPECT_FALSE(rights.black_queenside);
  SCOPED_TRACE(log_board_to_string(board));
}

TEST(BoardTest, BlackKingRookMoveClearsKingsideRight) {
  Board board = board_from_fen("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1");

  UndoState undo;
  const Move move(square_from_algebraic('h', '8'),
                  square_from_algebraic('g', '8'), PieceType::None,
                  MoveFlag::Quiet);
  board.make_move(move, undo);

  expect_positions_identical(
      board, board_from_fen("r3k1r1/8/8/8/8/8/8/R3K2R w KQq - 1 2"));
  const CastlingRights rights = board.castling_rights();
  EXPECT_FALSE(rights.black_kingside);
  EXPECT_TRUE(rights.black_queenside);
  SCOPED_TRACE(log_board_to_string(board));
}

// Capturing a rook on its home square must also remove the opponent's matching
// castling right.
TEST(BoardTest, CapturingRookClearsOpponentCastlingRight) {
  Board board = board_from_fen("4k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1");
  const Board snapshot = board;

  UndoState undo;
  const Move move(square_from_algebraic('h', '8'),
                  square_from_algebraic('h', '1'), PieceType::None,
                  MoveFlag::Capture);
  board.make_move(move, undo);

  const CastlingRights rights = board.castling_rights();
  EXPECT_FALSE(rights.white_kingside);
  EXPECT_TRUE(rights.white_queenside);
  EXPECT_FALSE(rights.black_kingside);
  EXPECT_FALSE(rights.black_queenside);

  board.unmake_move(undo);
  expect_positions_identical(board, snapshot);
  SCOPED_TRACE(log_board_to_string(board));
}

// ---------------------------------------------------------------------------
// Transposition: different move orders reaching the same position
// ---------------------------------------------------------------------------

TEST(BoardTest, TranspositionProducesIdenticalHash) {
  UndoState undo;

  Board first = Board::starting_position();
  first.make_move(Move(square_from_algebraic('g', '1'),
                       square_from_algebraic('f', '3'), PieceType::None,
                       MoveFlag::Quiet),
                  undo);
  first.make_move(Move(square_from_algebraic('g', '8'),
                       square_from_algebraic('f', '6'), PieceType::None,
                       MoveFlag::Quiet),
                  undo);
  first.make_move(Move(square_from_algebraic('b', '1'),
                       square_from_algebraic('c', '3'), PieceType::None,
                       MoveFlag::Quiet),
                  undo);
  first.make_move(Move(square_from_algebraic('b', '8'),
                       square_from_algebraic('c', '6'), PieceType::None,
                       MoveFlag::Quiet),
                  undo);

  Board second = Board::starting_position();
  second.make_move(Move(square_from_algebraic('b', '1'),
                        square_from_algebraic('c', '3'), PieceType::None,
                        MoveFlag::Quiet),
                   undo);
  second.make_move(Move(square_from_algebraic('b', '8'),
                        square_from_algebraic('c', '6'), PieceType::None,
                        MoveFlag::Quiet),
                   undo);
  second.make_move(Move(square_from_algebraic('g', '1'),
                        square_from_algebraic('f', '3'), PieceType::None,
                        MoveFlag::Quiet),
                   undo);
  second.make_move(Move(square_from_algebraic('g', '8'),
                        square_from_algebraic('f', '6'), PieceType::None,
                        MoveFlag::Quiet),
                   undo);

  log_board(first, std::clog);
  log_board(second, std::clog);

  EXPECT_EQ(first.zobrist_key(), second.zobrist_key());
  expect_positions_identical(first, second);
  SCOPED_TRACE(log_board_to_string(first));
}

TEST(BoardTest, LongerTranspositionProducesIdenticalHash) {
  UndoState undo;

  std::vector<Move> moveOrder1 = parse_pgn_moves(
      "1. d4 Nf6 2. c4 e6 3. Nc3 Bb4 4. Qc2 d5 5. cxd5 exd5 6. Bg5 h6");
  std::vector<Move> moveOrder2 = parse_pgn_moves(
      "1. d4 d5 2. c4 e6 3. Nc3 Nf6 4. cxd5 exd5 5. Bg5 Bb4 6. Qc2 h6");
  Board first = Board::starting_position();
  Board second = Board::starting_position();
  for (Move move : moveOrder1) {
    first.make_move(move, undo);
  }

  for (Move move : moveOrder2) {
    second.make_move(move, undo);
  }

  log_board(first, std::clog, true);
  log_board(second, std::clog, true);

  EXPECT_EQ(first.zobrist_key(), second.zobrist_key());
  expect_positions_identical(first, second);
  SCOPED_TRACE(log_board_to_string(first));
}

} // namespace
} // namespace chesspp::core
