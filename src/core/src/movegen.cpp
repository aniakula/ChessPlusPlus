#include "movegen.hpp"
#include "board.hpp"
#include "types.hpp"

#include <bit>

namespace chesspp::core {

namespace {

[[nodiscard]] Square pop_lsb(Bitboard &bitboard) noexcept {
  const Square square = static_cast<Square>(std::countr_zero(bitboard));
  bitboard &= bitboard - 1;
  return square;
}

[[nodiscard]] bool is_capture(MoveFlag flags) noexcept {
  return has_flag(flags, MoveFlag::Capture);
}

[[nodiscard]] bool should_emit(MoveGenerationType type,
                               MoveFlag flags) noexcept {
  switch (type) {
  case MoveGenerationType::CapturesOnly:
    return is_capture(flags);
  case MoveGenerationType::QuietOnly:
    return !is_capture(flags);
  case MoveGenerationType::All:
  // place holder: Optimize to only check evasion later
  case MoveGenerationType::Evasions:
    return true;
  }

  return true;
}

void push_move(MoveList &moves, MoveGenerationType type, Square from, Square to,
               MoveFlag flags = MoveFlag::Quiet) noexcept {
  if (should_emit(type, flags)) {
    moves.push(Move(from, to, PieceType::None, flags));
  }
}

void push_promotions(MoveList &moves, MoveGenerationType type, Square from,
                     Square to, MoveFlag flags) noexcept {
  if (!should_emit(type, flags)) {
    return;
  }

  moves.push(Move(from, to, PieceType::Queen, flags));
  moves.push(Move(from, to, PieceType::Rook, flags));
  moves.push(Move(from, to, PieceType::Bishop, flags));
  moves.push(Move(from, to, PieceType::Knight, flags));
}

void push_piece_targets(MoveList &moves, MoveGenerationType type, Square from,
                        Bitboard targets, Bitboard enemy) noexcept {
  while (targets != 0) {
    const Square to = pop_lsb(targets);
    const MoveFlag flags = (MoveGenerator::square_mask(to) & enemy) != 0
                               ? MoveFlag::Capture
                               : MoveFlag::Quiet;
    push_move(moves, type, from, to, flags);
  }
}

[[nodiscard]] bool has_piece_on(const Board &board, Square square, Color color,
                                PieceType piece) noexcept {
  return (board.pieces(color, piece) & MoveGenerator::square_mask(square)) != 0;
}

} // namespace

void MoveList::clear() noexcept { size_ = 0; }

void MoveList::push(Move move) noexcept {
  // TODO: In debug builds, assert size_ < kMaxMovesPerPosition.
  moves_[size_] = move;
  ++size_;
}

std::size_t MoveList::size() const noexcept { return size_; }

bool MoveList::empty() const noexcept { return size_ == 0; }

const Move &MoveList::operator[](std::size_t index) const noexcept {
  return moves_[index];
}

MoveList::const_iterator MoveList::begin() const noexcept {
  return moves_.begin();
}

MoveList::const_iterator MoveList::end() const noexcept {
  return moves_.begin() + static_cast<std::ptrdiff_t>(size_);
}

bool MoveList::contains(const Move &move) const {
  for (const Move &candidate : *this) {
    if (candidate == move) {
      return true;
    }
  }

  return false;
}

Bitboard MoveGenerator::square_mask(Square square) noexcept {
  return Bitboard{1} << square;
}

Bitboard MoveGenerator::ray_attacks(Square square, int dy, int dx,
                                    Bitboard occupancy) noexcept {
  Bitboard attacks = 0;
  int rank = square / 8;
  int file = square % 8;
  for (rank += dy, file += dx; rank >= 0 && rank < 8 && file >= 0 && file < 8;
       rank += dy, file += dx) {
    Square target = static_cast<Square>(rank * 8 + file);
    Bitboard mask = square_mask(target);
    attacks |= mask;
    if (occupancy & mask) {
      break;
    }
  }
  return attacks;
}

void MoveGenerator::generate_pseudo_legal(const Board &board, MoveList &moves,
                                          MoveGenerationType type) noexcept {
  moves.clear();

  generate_pseudo_legal_pawns(board, moves, type);
  generate_pseudo_legal_knights(board, moves, type);
  generate_pseudo_legal_bishops(board, moves, type);
  generate_pseudo_legal_rooks(board, moves, type);
  generate_pseudo_legal_queens(board, moves, type);
  generate_pseudo_legal_king(board, moves, type);
}

void MoveGenerator::generate_legal(Board &board, MoveList &moves) noexcept {
  MoveList pseudo_legal;
  generate_pseudo_legal(board, pseudo_legal);

  moves.clear();
  for (const Move &move : pseudo_legal) {
    if (leaves_king_safe(board, move)) {
      moves.push(move);
    }
  }
}

Bitboard MoveGenerator::pawn_attacks(Color color, Square square) noexcept {
  if (color == Color::Black) {
    return black_pawn_attack_table[square];
  }

  return white_pawn_attack_table[square];
}

Bitboard MoveGenerator::knight_attacks(Square square) noexcept {
  return knight_attack_table[square];
}

Bitboard MoveGenerator::bishop_attacks(Square square,
                                       Bitboard occupancy) noexcept {
  return ray_attacks(square, -1, -1, occupancy) |
         ray_attacks(square, -1, 1, occupancy) |
         ray_attacks(square, 1, -1, occupancy) |
         ray_attacks(square, 1, 1, occupancy);
}

Bitboard MoveGenerator::rook_attacks(Square square,
                                     Bitboard occupancy) noexcept {
  return ray_attacks(square, 0, -1, occupancy) |
         ray_attacks(square, 0, 1, occupancy) |
         ray_attacks(square, -1, 0, occupancy) |
         ray_attacks(square, 1, 0, occupancy);
}

Bitboard MoveGenerator::queen_attacks(Square square,
                                      Bitboard occupancy) noexcept {
  return bishop_attacks(square, occupancy) | rook_attacks(square, occupancy);
}

Bitboard MoveGenerator::king_attacks(Square square) noexcept {
  return king_attack_table[square];
}

void MoveGenerator::generate_pseudo_legal_pawns(
    const Board &board, MoveList &moves, MoveGenerationType type) noexcept {
  const Color us = board.side_to_move();
  const Color them = opposite(us);
  const Bitboard occupied = board.all_occupancy();
  const Bitboard enemy = board.occupancy(them);
  Bitboard pawns = board.pieces(us, PieceType::Pawn);

  const int push_delta = us == Color::White ? 8 : -8;
  const int start_rank = us == Color::White ? 1 : 6;
  const int promotion_rank = us == Color::White ? 7 : 0;

  while (pawns != 0) {
    const Square from = pop_lsb(pawns);
    const int from_rank = from / 8;
    const int one_step = static_cast<int>(from) + push_delta;

    if (one_step >= 0 && one_step < SQUARE_COUNT) {
      const Square to = static_cast<Square>(one_step);
      const Bitboard to_mask = square_mask(to);
      if ((occupied & to_mask) == 0) {
        if ((to / 8) == promotion_rank) {
          push_promotions(moves, type, from, to, MoveFlag::Promotion);
        } else {
          push_move(moves, type, from, to);
        }

        const int two_step = static_cast<int>(from) + (push_delta * 2);
        if (from_rank == start_rank && two_step >= 0 &&
            two_step < SQUARE_COUNT) {
          const Square double_push_to = static_cast<Square>(two_step);
          if ((occupied & square_mask(double_push_to)) == 0) {
            push_move(moves, type, from, double_push_to,
                      MoveFlag::DoublePawnPush);
          }
        }
      }
    }

    Bitboard captures = pawn_attacks(us, from) & enemy;
    while (captures != 0) {
      const Square to = pop_lsb(captures);
      if ((to / 8) == promotion_rank) {
        push_promotions(moves, type, from, to,
                        MoveFlag::Promotion | MoveFlag::Capture);
      } else {
        push_move(moves, type, from, to, MoveFlag::Capture);
      }
    }

    const Square en_passant_square = board.en_passant_square();
    if (en_passant_square != NO_SQUARE &&
        (pawn_attacks(us, from) & square_mask(en_passant_square)) != 0) {
      push_move(moves, type, from, en_passant_square,
                MoveFlag::EnPassant | MoveFlag::Capture);
    }
  }
}

void MoveGenerator::generate_pseudo_legal_knights(
    const Board &board, MoveList &moves, MoveGenerationType type) noexcept {
  const Color us = board.side_to_move();
  const Color them = opposite(us);
  const Bitboard friendly = board.occupancy(us);
  const Bitboard enemy = board.occupancy(them);
  Bitboard knights = board.pieces(us, PieceType::Knight);

  while (knights != 0) {
    const Square from = pop_lsb(knights);
    const Bitboard targets = knight_attacks(from) & ~friendly;
    push_piece_targets(moves, type, from, targets, enemy);
  }
}

void MoveGenerator::generate_pseudo_legal_bishops(
    const Board &board, MoveList &moves, MoveGenerationType type) noexcept {
  const Color us = board.side_to_move();
  const Color them = opposite(us);
  const Bitboard friendly = board.occupancy(us);
  const Bitboard enemy = board.occupancy(them);
  const Bitboard occupied = board.all_occupancy();
  Bitboard bishops = board.pieces(us, PieceType::Bishop);

  while (bishops != 0) {
    const Square from = pop_lsb(bishops);
    const Bitboard targets = bishop_attacks(from, occupied) & ~friendly;
    push_piece_targets(moves, type, from, targets, enemy);
  }
}

void MoveGenerator::generate_pseudo_legal_rooks(
    const Board &board, MoveList &moves, MoveGenerationType type) noexcept {
  const Color us = board.side_to_move();
  const Color them = opposite(us);
  const Bitboard friendly = board.occupancy(us);
  const Bitboard enemy = board.occupancy(them);
  const Bitboard occupied = board.all_occupancy();
  Bitboard rooks = board.pieces(us, PieceType::Rook);

  while (rooks != 0) {
    const Square from = pop_lsb(rooks);
    const Bitboard targets = rook_attacks(from, occupied) & ~friendly;
    push_piece_targets(moves, type, from, targets, enemy);
  }
}

void MoveGenerator::generate_pseudo_legal_queens(
    const Board &board, MoveList &moves, MoveGenerationType type) noexcept {
  const Color us = board.side_to_move();
  const Color them = opposite(us);
  const Bitboard friendly = board.occupancy(us);
  const Bitboard enemy = board.occupancy(them);
  const Bitboard occupied = board.all_occupancy();
  Bitboard queens = board.pieces(us, PieceType::Queen);

  while (queens != 0) {
    const Square from = pop_lsb(queens);
    const Bitboard targets = queen_attacks(from, occupied) & ~friendly;
    push_piece_targets(moves, type, from, targets, enemy);
  }
}

void MoveGenerator::generate_pseudo_legal_king(
    const Board &board, MoveList &moves, MoveGenerationType type) noexcept {
  const Color us = board.side_to_move();
  const Color them = opposite(us);
  const Bitboard friendly = board.occupancy(us);
  const Bitboard enemy = board.occupancy(them);
  Bitboard kings = board.pieces(us, PieceType::King);

  while (kings != 0) {
    const Square from = pop_lsb(kings);
    const Bitboard targets = king_attacks(from) & ~friendly;
    push_piece_targets(moves, type, from, targets, enemy);
  }

  if (!should_emit(type, MoveFlag::Quiet) || board.in_check(us)) {
    return;
  }

  const CastlingRights rights = board.castling_rights();
  const Bitboard occupied = board.all_occupancy();
  if (us == Color::White) {
    const Square e1 = square_from_algebraic('e', '1');
    const Square f1 = square_from_algebraic('f', '1');
    const Square g1 = square_from_algebraic('g', '1');
    const Square d1 = square_from_algebraic('d', '1');
    const Square c1 = square_from_algebraic('c', '1');
    const Square b1 = square_from_algebraic('b', '1');
    const Square h1 = square_from_algebraic('h', '1');
    const Square a1 = square_from_algebraic('a', '1');

    if (rights.white_kingside && has_piece_on(board, e1, us, PieceType::King) &&
        has_piece_on(board, h1, us, PieceType::Rook) &&
        (occupied & (square_mask(f1) | square_mask(g1))) == 0 &&
        board.attackers_to(f1, them) == 0 &&
        board.attackers_to(g1, them) == 0) {
      moves.push(Move(e1, g1, PieceType::None, MoveFlag::KingCastle));
    }

    if (rights.white_queenside &&
        has_piece_on(board, e1, us, PieceType::King) &&
        has_piece_on(board, a1, us, PieceType::Rook) &&
        (occupied & (square_mask(d1) | square_mask(c1) | square_mask(b1))) ==
            0 &&
        board.attackers_to(d1, them) == 0 &&
        board.attackers_to(c1, them) == 0) {
      moves.push(Move(e1, c1, PieceType::None, MoveFlag::QueenCastle));
    }
  } else {
    const Square e8 = square_from_algebraic('e', '8');
    const Square f8 = square_from_algebraic('f', '8');
    const Square g8 = square_from_algebraic('g', '8');
    const Square d8 = square_from_algebraic('d', '8');
    const Square c8 = square_from_algebraic('c', '8');
    const Square b8 = square_from_algebraic('b', '8');
    const Square h8 = square_from_algebraic('h', '8');
    const Square a8 = square_from_algebraic('a', '8');

    if (rights.black_kingside && has_piece_on(board, e8, us, PieceType::King) &&
        has_piece_on(board, h8, us, PieceType::Rook) &&
        (occupied & (square_mask(f8) | square_mask(g8))) == 0 &&
        board.attackers_to(f8, them) == 0 &&
        board.attackers_to(g8, them) == 0) {
      moves.push(Move(e8, g8, PieceType::None, MoveFlag::KingCastle));
    }

    if (rights.black_queenside &&
        has_piece_on(board, e8, us, PieceType::King) &&
        has_piece_on(board, a8, us, PieceType::Rook) &&
        (occupied & (square_mask(d8) | square_mask(c8) | square_mask(b8))) ==
            0 &&
        board.attackers_to(d8, them) == 0 &&
        board.attackers_to(c8, them) == 0) {
      moves.push(Move(e8, c8, PieceType::None, MoveFlag::QueenCastle));
    }
  }
}

bool MoveGenerator::leaves_king_safe(Board &board, Move move) noexcept {
  bool inCheck = true;
  Color turnPlayer = board.side_to_move();
  UndoState currState{};
  board.make_move(move, currState);
  if (board.in_check(turnPlayer)) {
    inCheck = false;
  }

  board.unmake_move(currState);
  return inCheck;
}

} // namespace chesspp::core
