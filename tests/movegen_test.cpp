#include "board.hpp"
#include "movegen.hpp"
#include "types.hpp"

#include <gtest/gtest.h>

#include <bit>
#include <cstddef>

namespace chesspp::core {
namespace {

Square sq(char file, char rank) { return square_from_algebraic(file, rank); }

Bitboard mask(char file, char rank) {
  return MoveGenerator::square_mask(sq(file, rank));
}

bool has_move(const MoveList &list, Square from, Square to,
              MoveFlag flags = MoveFlag::Quiet,
              PieceType promotion = PieceType::None) {
  return list.contains(Move(from, to, promotion, flags));
}

std::size_t count_from(const MoveList &list, Square from) {
  std::size_t count = 0;
  for (const Move &move : list) {
    if (move.from() == from) {
      ++count;
    }
  }
  return count;
}

bool all_moves_have_capture_flag(const MoveList &list) {
  for (const Move &move : list) {
    if (!has_flag(move.flags(), MoveFlag::Capture)) {
      return false;
    }
  }
  return true;
}

bool no_moves_have_capture_flag(const MoveList &list) {
  for (const Move &move : list) {
    if (has_flag(move.flags(), MoveFlag::Capture)) {
      return false;
    }
  }
  return true;
}

// ---------------------------------------------------------------------------
// square_mask
// ---------------------------------------------------------------------------

TEST(MoveGenAttackTest, SquareMaskSetsExactlyOneBit) {
  EXPECT_EQ(MoveGenerator::square_mask(0), Bitboard{1});
  EXPECT_EQ(MoveGenerator::square_mask(63), Bitboard{1} << 63);
  EXPECT_EQ(MoveGenerator::square_mask(sq('e', '4')), Bitboard{1} << 28);
}

// ---------------------------------------------------------------------------
// knight_attacks
// ---------------------------------------------------------------------------

TEST(MoveGenAttackTest, KnightAttacksCorners) {
  EXPECT_EQ(MoveGenerator::knight_attacks(sq('a', '1')),
            mask('b', '3') | mask('c', '2'));
  EXPECT_EQ(MoveGenerator::knight_attacks(sq('h', '8')),
            mask('g', '6') | mask('f', '7'));
  EXPECT_EQ(std::popcount(MoveGenerator::knight_attacks(sq('a', '1'))), 2);
}

TEST(MoveGenAttackTest, KnightAttacksCenterHasEightTargets) {
  const Bitboard expected = mask('b', '3') | mask('b', '5') | mask('c', '2') |
                            mask('c', '6') | mask('e', '2') | mask('e', '6') |
                            mask('f', '3') | mask('f', '5');
  EXPECT_EQ(MoveGenerator::knight_attacks(sq('d', '4')), expected);
  EXPECT_EQ(std::popcount(MoveGenerator::knight_attacks(sq('d', '4'))), 8);
}

// ---------------------------------------------------------------------------
// king_attacks
// ---------------------------------------------------------------------------

TEST(MoveGenAttackTest, KingAttacksEdgeCenterAndCorner) {
  EXPECT_EQ(MoveGenerator::king_attacks(sq('e', '1')),
            mask('d', '1') | mask('f', '1') | mask('d', '2') | mask('e', '2') |
                mask('f', '2'));
  EXPECT_EQ(std::popcount(MoveGenerator::king_attacks(sq('e', '1'))), 5);

  EXPECT_EQ(MoveGenerator::king_attacks(sq('a', '1')),
            mask('a', '2') | mask('b', '1') | mask('b', '2'));
  EXPECT_EQ(std::popcount(MoveGenerator::king_attacks(sq('a', '1'))), 3);

  EXPECT_EQ(std::popcount(MoveGenerator::king_attacks(sq('d', '4'))), 8);
}

// ---------------------------------------------------------------------------
// pawn_attacks
// ---------------------------------------------------------------------------

TEST(MoveGenAttackTest, PawnAttacksByColorAndEdges) {
  EXPECT_EQ(MoveGenerator::pawn_attacks(Color::White, sq('e', '4')),
            mask('d', '5') | mask('f', '5'));
  EXPECT_EQ(MoveGenerator::pawn_attacks(Color::Black, sq('e', '5')),
            mask('d', '4') | mask('f', '4'));
  // Edge files only attack one diagonal.
  EXPECT_EQ(MoveGenerator::pawn_attacks(Color::White, sq('a', '2')),
            mask('b', '3'));
  EXPECT_EQ(MoveGenerator::pawn_attacks(Color::Black, sq('h', '7')),
            mask('g', '6'));
}

// ---------------------------------------------------------------------------
// ray_attacks
// ---------------------------------------------------------------------------

TEST(MoveGenAttackTest, RayAttacksToEdgeWhenEmpty) {
  const Bitboard up = MoveGenerator::ray_attacks(sq('d', '4'), 1, 0, 0);
  EXPECT_EQ(up, mask('d', '5') | mask('d', '6') | mask('d', '7') | mask('d', '8'));
}

TEST(MoveGenAttackTest, RayAttacksStopsAtBlockerInclusive) {
  const Bitboard occupancy = mask('d', '6');
  const Bitboard up = MoveGenerator::ray_attacks(sq('d', '4'), 1, 0, occupancy);
  EXPECT_EQ(up, mask('d', '5') | mask('d', '6'));
}

// ---------------------------------------------------------------------------
// bishop / rook / queen attacks
// ---------------------------------------------------------------------------

TEST(MoveGenAttackTest, BishopRookQueenEmptyBoardCounts) {
  EXPECT_EQ(std::popcount(MoveGenerator::bishop_attacks(sq('d', '4'), 0)), 13);
  EXPECT_EQ(std::popcount(MoveGenerator::rook_attacks(sq('d', '4'), 0)), 14);
  EXPECT_EQ(std::popcount(MoveGenerator::queen_attacks(sq('d', '4'), 0)), 27);

  EXPECT_EQ(std::popcount(MoveGenerator::bishop_attacks(sq('a', '1'), 0)), 7);
  EXPECT_EQ(std::popcount(MoveGenerator::rook_attacks(sq('a', '1'), 0)), 14);
}

TEST(MoveGenAttackTest, BishopAttacksBlockedInclusive) {
  const Bitboard occupancy = mask('f', '6');
  const Bitboard attacks = MoveGenerator::bishop_attacks(sq('d', '4'), occupancy);
  EXPECT_EQ(std::popcount(attacks), 11);
  EXPECT_NE(attacks & mask('f', '6'), 0U);   // blocker square reachable
  EXPECT_EQ(attacks & mask('g', '7'), 0U);   // beyond blocker is not
  EXPECT_EQ(attacks & mask('h', '8'), 0U);
}

// ---------------------------------------------------------------------------
// Knight generation
// ---------------------------------------------------------------------------

TEST(MoveGenPieceTest, KnightQuietMovesOnOpenBoard) {
  const Board board = Board::from_fen("4k3/8/8/8/3N4/8/8/4K3 w - - 0 1");
  MoveList moves;
  MoveGenerator::generate_pseudo_legal_knights(board, moves,
                                               MoveGenerationType::All);

  EXPECT_EQ(count_from(moves, sq('d', '4')), 8U);
  for (const char *target : {"b3", "b5", "c2", "c6", "e2", "e6", "f3", "f5"}) {
    EXPECT_TRUE(has_move(moves, sq('d', '4'), sq(target[0], target[1])))
        << "missing quiet knight move to " << target;
  }
}

TEST(MoveGenPieceTest, KnightCaptureFlaggedCorrectly) {
  const Board board = Board::from_fen("4k3/8/4p3/8/3N4/8/8/4K3 w - - 0 1");
  MoveList moves;
  MoveGenerator::generate_pseudo_legal_knights(board, moves,
                                               MoveGenerationType::All);

  EXPECT_EQ(count_from(moves, sq('d', '4')), 8U);
  EXPECT_TRUE(has_move(moves, sq('d', '4'), sq('e', '6'), MoveFlag::Capture));
  EXPECT_FALSE(has_move(moves, sq('d', '4'), sq('e', '6'), MoveFlag::Quiet));
  // A non-capturing target stays quiet.
  EXPECT_TRUE(has_move(moves, sq('d', '4'), sq('f', '5'), MoveFlag::Quiet));
}

TEST(MoveGenPieceTest, KnightBlockedByFriendlyPiece) {
  const Board board = Board::from_fen("4k3/8/4P3/8/3N4/8/8/4K3 w - - 0 1");
  MoveList moves;
  MoveGenerator::generate_pseudo_legal_knights(board, moves,
                                               MoveGenerationType::All);

  EXPECT_EQ(count_from(moves, sq('d', '4')), 7U);
  EXPECT_FALSE(has_move(moves, sq('d', '4'), sq('e', '6'), MoveFlag::Quiet));
  EXPECT_FALSE(has_move(moves, sq('d', '4'), sq('e', '6'), MoveFlag::Capture));
}

TEST(MoveGenPieceTest, GenerationTypeFiltersCapturesAndQuiets) {
  const Board board = Board::from_fen("4k3/8/4p3/8/3N4/8/8/4K3 w - - 0 1");

  MoveList all;
  MoveGenerator::generate_pseudo_legal_knights(board, all,
                                               MoveGenerationType::All);
  EXPECT_EQ(all.size(), 8U);

  MoveList captures;
  MoveGenerator::generate_pseudo_legal_knights(board, captures,
                                               MoveGenerationType::CapturesOnly);
  ASSERT_EQ(captures.size(), 1U);
  EXPECT_TRUE(has_move(captures, sq('d', '4'), sq('e', '6'), MoveFlag::Capture));

  MoveList quiets;
  MoveGenerator::generate_pseudo_legal_knights(board, quiets,
                                               MoveGenerationType::QuietOnly);
  EXPECT_EQ(quiets.size(), 7U);
  EXPECT_TRUE(no_moves_have_capture_flag(quiets));

  MoveList evasions;
  MoveGenerator::generate_pseudo_legal_knights(board, evasions,
                                               MoveGenerationType::Evasions);
  EXPECT_EQ(evasions.size(), 8U); // Evasions currently behaves like All.
}

// ---------------------------------------------------------------------------
// Bishop / Rook / Queen generation
// ---------------------------------------------------------------------------

TEST(MoveGenPieceTest, BishopOpenBoardAndCapture) {
  const Board open = Board::from_fen("4k3/8/8/8/3B4/8/8/4K3 w - - 0 1");
  MoveList open_moves;
  MoveGenerator::generate_pseudo_legal_bishops(open, open_moves,
                                               MoveGenerationType::All);
  EXPECT_EQ(count_from(open_moves, sq('d', '4')), 13U);

  const Board blocked = Board::from_fen("4k3/6p1/8/8/3B4/8/8/4K3 w - - 0 1");
  MoveList blocked_moves;
  MoveGenerator::generate_pseudo_legal_bishops(blocked, blocked_moves,
                                               MoveGenerationType::All);
  EXPECT_EQ(count_from(blocked_moves, sq('d', '4')), 12U);
  EXPECT_TRUE(has_move(blocked_moves, sq('d', '4'), sq('g', '7'),
                       MoveFlag::Capture));
  EXPECT_FALSE(has_move(blocked_moves, sq('d', '4'), sq('h', '8')));
  EXPECT_TRUE(has_move(blocked_moves, sq('d', '4'), sq('f', '6'),
                       MoveFlag::Quiet));
}

TEST(MoveGenPieceTest, RookOpenBoardAndCapture) {
  const Board open = Board::from_fen("4k3/8/8/8/3R4/8/8/4K3 w - - 0 1");
  MoveList open_moves;
  MoveGenerator::generate_pseudo_legal_rooks(open, open_moves,
                                             MoveGenerationType::All);
  EXPECT_EQ(count_from(open_moves, sq('d', '4')), 14U);

  const Board blocked = Board::from_fen("4k3/3p4/8/8/3R4/8/8/4K3 w - - 0 1");
  MoveList blocked_moves;
  MoveGenerator::generate_pseudo_legal_rooks(blocked, blocked_moves,
                                             MoveGenerationType::All);
  EXPECT_EQ(count_from(blocked_moves, sq('d', '4')), 13U);
  EXPECT_TRUE(has_move(blocked_moves, sq('d', '4'), sq('d', '7'),
                       MoveFlag::Capture));
  EXPECT_FALSE(has_move(blocked_moves, sq('d', '4'), sq('d', '8')));
}

TEST(MoveGenPieceTest, QueenOpenBoardCombinesBishopAndRook) {
  const Board board = Board::from_fen("4k3/8/8/8/3Q4/8/8/4K3 w - - 0 1");
  MoveList moves;
  MoveGenerator::generate_pseudo_legal_queens(board, moves,
                                              MoveGenerationType::All);
  EXPECT_EQ(count_from(moves, sq('d', '4')), 27U);
}

// ---------------------------------------------------------------------------
// King generation (including castling)
// ---------------------------------------------------------------------------

TEST(MoveGenPieceTest, KingQuietMovesNoCastlingRights) {
  const Board board = Board::from_fen("4k3/8/8/8/8/8/8/4K3 w - - 0 1");
  MoveList moves;
  MoveGenerator::generate_pseudo_legal_king(board, moves,
                                            MoveGenerationType::All);
  EXPECT_EQ(moves.size(), 5U);
  for (const char *target : {"d1", "f1", "d2", "e2", "f2"}) {
    EXPECT_TRUE(has_move(moves, sq('e', '1'), sq(target[0], target[1])));
  }
}

TEST(MoveGenPieceTest, KingCastlesBothSidesWhenLegal) {
  const Board board =
      Board::from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
  MoveList moves;
  MoveGenerator::generate_pseudo_legal_king(board, moves,
                                            MoveGenerationType::All);
  EXPECT_EQ(moves.size(), 7U);
  EXPECT_TRUE(has_move(moves, sq('e', '1'), sq('g', '1'), MoveFlag::KingCastle));
  EXPECT_TRUE(has_move(moves, sq('e', '1'), sq('c', '1'), MoveFlag::QueenCastle));
}

TEST(MoveGenPieceTest, KingCastleBlockedWhenTransitSquareAttacked) {
  // Black rook on f7 attacks f1, so kingside castling is forbidden, but
  // queenside remains legal.
  const Board board = Board::from_fen("4k3/5r2/8/8/8/8/8/R3K2R w KQ - 0 1");
  MoveList moves;
  MoveGenerator::generate_pseudo_legal_king(board, moves,
                                            MoveGenerationType::All);
  EXPECT_FALSE(has_move(moves, sq('e', '1'), sq('g', '1'), MoveFlag::KingCastle));
  EXPECT_TRUE(has_move(moves, sq('e', '1'), sq('c', '1'), MoveFlag::QueenCastle));
}

TEST(MoveGenPieceTest, KingDoesNotCastleWhileInCheck) {
  // Black rook on e2 checks the white king, so no castling is generated.
  const Board board = Board::from_fen("4k3/8/8/8/8/8/4r3/R3K2R w KQ - 0 1");
  MoveList moves;
  MoveGenerator::generate_pseudo_legal_king(board, moves,
                                            MoveGenerationType::All);
  EXPECT_FALSE(has_move(moves, sq('e', '1'), sq('g', '1'), MoveFlag::KingCastle));
  EXPECT_FALSE(has_move(moves, sq('e', '1'), sq('c', '1'), MoveFlag::QueenCastle));
  EXPECT_TRUE(has_move(moves, sq('e', '1'), sq('e', '2'), MoveFlag::Capture));
}

TEST(MoveGenPieceTest, KingNoCastleWithoutRights) {
  const Board board = Board::from_fen("r3k2r/8/8/8/8/8/8/R3K2R w - - 0 1");
  MoveList moves;
  MoveGenerator::generate_pseudo_legal_king(board, moves,
                                            MoveGenerationType::All);
  EXPECT_EQ(moves.size(), 5U);
  EXPECT_FALSE(has_move(moves, sq('e', '1'), sq('g', '1'), MoveFlag::KingCastle));
  EXPECT_FALSE(has_move(moves, sq('e', '1'), sq('c', '1'), MoveFlag::QueenCastle));
}

// ---------------------------------------------------------------------------
// Pawn generation
// ---------------------------------------------------------------------------

TEST(MoveGenPieceTest, PawnSingleAndDoublePush) {
  const Board board = Board::from_fen("4k3/8/8/8/8/8/4P3/4K3 w - - 0 1");
  MoveList moves;
  MoveGenerator::generate_pseudo_legal_pawns(board, moves,
                                             MoveGenerationType::All);
  EXPECT_EQ(count_from(moves, sq('e', '2')), 2U);
  EXPECT_TRUE(has_move(moves, sq('e', '2'), sq('e', '3'), MoveFlag::Quiet));
  EXPECT_TRUE(
      has_move(moves, sq('e', '2'), sq('e', '4'), MoveFlag::DoublePawnPush));
}

TEST(MoveGenPieceTest, PawnSinglePushBlockedRemovesAllPushes) {
  const Board board = Board::from_fen("4k3/8/8/8/8/4n3/4P3/4K3 w - - 0 1");
  MoveList moves;
  MoveGenerator::generate_pseudo_legal_pawns(board, moves,
                                             MoveGenerationType::All);
  EXPECT_EQ(count_from(moves, sq('e', '2')), 0U);
}

TEST(MoveGenPieceTest, PawnDoublePushBlockedKeepsSinglePush) {
  const Board board = Board::from_fen("4k3/8/8/8/4n3/8/4P3/4K3 w - - 0 1");
  MoveList moves;
  MoveGenerator::generate_pseudo_legal_pawns(board, moves,
                                             MoveGenerationType::All);
  EXPECT_EQ(count_from(moves, sq('e', '2')), 1U);
  EXPECT_TRUE(has_move(moves, sq('e', '2'), sq('e', '3'), MoveFlag::Quiet));
  EXPECT_FALSE(
      has_move(moves, sq('e', '2'), sq('e', '4'), MoveFlag::DoublePawnPush));
}

TEST(MoveGenPieceTest, PawnCapturesBothDiagonals) {
  const Board board = Board::from_fen("4k3/8/8/8/8/3n1n2/4P3/4K3 w - - 0 1");
  MoveList moves;
  MoveGenerator::generate_pseudo_legal_pawns(board, moves,
                                             MoveGenerationType::All);
  EXPECT_EQ(count_from(moves, sq('e', '2')), 4U);
  EXPECT_TRUE(has_move(moves, sq('e', '2'), sq('d', '3'), MoveFlag::Capture));
  EXPECT_TRUE(has_move(moves, sq('e', '2'), sq('f', '3'), MoveFlag::Capture));
  EXPECT_TRUE(has_move(moves, sq('e', '2'), sq('e', '3'), MoveFlag::Quiet));
  EXPECT_TRUE(
      has_move(moves, sq('e', '2'), sq('e', '4'), MoveFlag::DoublePawnPush));
}

TEST(MoveGenPieceTest, PawnPromotionGeneratesAllFourPieces) {
  const Board board = Board::from_fen("4k3/P7/8/8/8/8/8/4K3 w - - 0 1");
  MoveList moves;
  MoveGenerator::generate_pseudo_legal_pawns(board, moves,
                                             MoveGenerationType::All);
  EXPECT_EQ(count_from(moves, sq('a', '7')), 4U);
  for (PieceType promo : {PieceType::Queen, PieceType::Rook, PieceType::Bishop,
                          PieceType::Knight}) {
    EXPECT_TRUE(has_move(moves, sq('a', '7'), sq('a', '8'), MoveFlag::Promotion,
                         promo));
  }
}

TEST(MoveGenPieceTest, PawnPromotionWithCapture) {
  const Board board = Board::from_fen("1n2k3/P7/8/8/8/8/8/4K3 w - - 0 1");
  MoveList moves;
  MoveGenerator::generate_pseudo_legal_pawns(board, moves,
                                             MoveGenerationType::All);
  EXPECT_EQ(count_from(moves, sq('a', '7')), 8U);
  for (PieceType promo : {PieceType::Queen, PieceType::Rook, PieceType::Bishop,
                          PieceType::Knight}) {
    EXPECT_TRUE(has_move(moves, sq('a', '7'), sq('a', '8'), MoveFlag::Promotion,
                         promo));
    EXPECT_TRUE(has_move(moves, sq('a', '7'), sq('b', '8'),
                         MoveFlag::Promotion | MoveFlag::Capture, promo));
  }
}

TEST(MoveGenPieceTest, PawnEnPassantCapture) {
  const Board board = Board::from_fen("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1");
  MoveList moves;
  MoveGenerator::generate_pseudo_legal_pawns(board, moves,
                                             MoveGenerationType::All);
  EXPECT_EQ(count_from(moves, sq('e', '5')), 2U);
  EXPECT_TRUE(has_move(moves, sq('e', '5'), sq('e', '6'), MoveFlag::Quiet));
  EXPECT_TRUE(has_move(moves, sq('e', '5'), sq('d', '6'),
                       MoveFlag::EnPassant | MoveFlag::Capture));
}

TEST(MoveGenPieceTest, BlackPawnPushesDownBoard) {
  const Board board = Board::from_fen("4k3/4p3/8/8/8/8/8/4K3 b - - 0 1");
  MoveList moves;
  MoveGenerator::generate_pseudo_legal_pawns(board, moves,
                                             MoveGenerationType::All);
  EXPECT_EQ(count_from(moves, sq('e', '7')), 2U);
  EXPECT_TRUE(has_move(moves, sq('e', '7'), sq('e', '6'), MoveFlag::Quiet));
  EXPECT_TRUE(
      has_move(moves, sq('e', '7'), sq('e', '5'), MoveFlag::DoublePawnPush));
}

// ---------------------------------------------------------------------------
// Full pseudo-legal / legal generation across whole positions
// ---------------------------------------------------------------------------

TEST(MoveGenFullTest, StartingPositionHasTwentyMoves) {
  const Board board = Board::starting_position();

  MoveList pseudo;
  MoveGenerator::generate_pseudo_legal(board, pseudo);
  EXPECT_EQ(pseudo.size(), 20U);

  Board copy = board;
  MoveList legal;
  MoveGenerator::generate_legal(copy, legal);
  EXPECT_EQ(legal.size(), 20U);

  EXPECT_TRUE(
      has_move(legal, sq('e', '2'), sq('e', '4'), MoveFlag::DoublePawnPush));
  EXPECT_TRUE(has_move(legal, sq('e', '2'), sq('e', '3'), MoveFlag::Quiet));
  EXPECT_TRUE(has_move(legal, sq('g', '1'), sq('f', '3'), MoveFlag::Quiet));
  EXPECT_TRUE(has_move(legal, sq('b', '1'), sq('c', '3'), MoveFlag::Quiet));
  EXPECT_TRUE(no_moves_have_capture_flag(legal));
}

TEST(MoveGenFullTest, CapturesOnlyAndQuietOnlyPartitionMoves) {
  // White pawn e4, black pawn d5: pawn can push (quiet) or capture; king has
  // only quiet moves.
  const Board board = Board::from_fen("4k3/8/8/3p4/4P3/8/8/4K3 w - - 0 1");

  MoveList all;
  MoveGenerator::generate_pseudo_legal(board, all, MoveGenerationType::All);
  EXPECT_EQ(all.size(), 7U);

  MoveList captures;
  MoveGenerator::generate_pseudo_legal(board, captures,
                                       MoveGenerationType::CapturesOnly);
  ASSERT_EQ(captures.size(), 1U);
  EXPECT_TRUE(all_moves_have_capture_flag(captures));
  EXPECT_TRUE(has_move(captures, sq('e', '4'), sq('d', '5'), MoveFlag::Capture));

  MoveList quiets;
  MoveGenerator::generate_pseudo_legal(board, quiets,
                                       MoveGenerationType::QuietOnly);
  EXPECT_EQ(quiets.size(), 6U);
  EXPECT_TRUE(no_moves_have_capture_flag(quiets));
}

TEST(MoveGenFullTest, LegalGenerationFiltersMovesIntoCheck) {
  // Black queen on e2 checks the white king on e1. The only legal reply is to
  // capture the queen.
  const Board board = Board::from_fen("4k3/8/8/8/8/8/4q3/4K3 w - - 0 1");

  MoveList pseudo_king;
  MoveGenerator::generate_pseudo_legal_king(board, pseudo_king,
                                            MoveGenerationType::All);
  EXPECT_EQ(pseudo_king.size(), 5U);

  Board copy = board;
  MoveList legal;
  MoveGenerator::generate_legal(copy, legal);
  ASSERT_EQ(legal.size(), 1U);
  EXPECT_TRUE(has_move(legal, sq('e', '1'), sq('e', '2'), MoveFlag::Capture));
}

TEST(MoveGenFullTest, PinnedPieceMayOnlyMoveAlongPin) {
  // White rook on e3 is pinned by the black rook on e8 against the king on e1.
  const Board board = Board::from_fen("4r1k1/8/8/8/8/4R3/8/4K3 w - - 0 1");

  Board copy = board;
  MoveList legal;
  MoveGenerator::generate_legal(copy, legal);

  // Moving along the pin file is fine, including capturing the pinner.
  EXPECT_TRUE(has_move(legal, sq('e', '3'), sq('e', '4'), MoveFlag::Quiet));
  EXPECT_TRUE(has_move(legal, sq('e', '3'), sq('e', '8'), MoveFlag::Capture));
  // Stepping off the file would expose the king.
  EXPECT_FALSE(has_move(legal, sq('e', '3'), sq('d', '3'), MoveFlag::Quiet));
  EXPECT_FALSE(has_move(legal, sq('e', '3'), sq('a', '3'), MoveFlag::Quiet));
}

// ---------------------------------------------------------------------------
// leaves_king_safe and attacker detection
// ---------------------------------------------------------------------------

TEST(MoveGenFullTest, LeavesKingSafeDetectsPinAndRestoresBoard) {
  Board board = Board::from_fen("4r1k1/8/8/8/8/4R3/8/4K3 w - - 0 1");
  const HashKey key_before = board.zobrist_key();
  const Color side_before = board.side_to_move();

  EXPECT_FALSE(MoveGenerator::leaves_king_safe(
      board, Move(sq('e', '3'), sq('d', '3'), PieceType::None, MoveFlag::Quiet)));
  EXPECT_TRUE(MoveGenerator::leaves_king_safe(
      board, Move(sq('e', '3'), sq('e', '4'), PieceType::None, MoveFlag::Quiet)));

  // The helper must make and unmake the move, leaving the board untouched.
  EXPECT_EQ(board.zobrist_key(), key_before);
  EXPECT_EQ(board.side_to_move(), side_before);
}

TEST(MoveGenFullTest, AttackersAndInCheckDetection) {
  const Board rook_check = Board::from_fen("4k3/8/8/8/4q3/8/8/4K3 w - - 0 1");
  EXPECT_TRUE(rook_check.in_check(Color::White));
  EXPECT_FALSE(rook_check.in_check(Color::Black));
  const Bitboard attackers =
      rook_check.attackers_to(sq('e', '1'), Color::Black);
  EXPECT_NE(attackers & mask('e', '4'), 0U);

  const Board knight_check =
      Board::from_fen("4k3/8/8/8/8/5n2/8/4K3 w - - 0 1");
  EXPECT_TRUE(knight_check.in_check(Color::White));
}

} // namespace
} // namespace chesspp::core
