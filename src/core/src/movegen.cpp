#include "movegen.hpp"

namespace chesspp::core {

void MoveList::clear() noexcept { size_ = 0; }

void MoveList::push(Move move) noexcept {
  // TODO: In debug builds, assert size_ < kMaxMovesPerPosition.
  moves_[size_] = move;
  ++size_;
}

std::size_t MoveList::size() const noexcept { return size_; }

bool MoveList::empty() const noexcept { return size_ == 0; }

const Move& MoveList::operator[](std::size_t index) const noexcept {
  return moves_[index];
}

MoveList::const_iterator MoveList::begin() const noexcept {
  return moves_.begin();
}

MoveList::const_iterator MoveList::end() const noexcept {
  return moves_.begin() + static_cast<std::ptrdiff_t>(size_);
}

void MoveGenerator::generate_pseudo_legal(const Board& board, MoveList& moves,
                                          MoveGenerationType type) noexcept {
  // TODO: Dispatch piece-specific bitboard generators.
  (void)board;
  (void)moves;
  (void)type;
}

void MoveGenerator::generate_legal(Board& board, MoveList& moves) noexcept {
  // TODO: Generate pseudo-legal moves, then make/unmake and keep king-safe moves.
  (void)board;
  (void)moves;
}

Bitboard MoveGenerator::pawn_attacks(Color color, Square square) noexcept {
  // TODO: Implement using shifts/masks or precomputed attack table.
  (void)color;
  (void)square;
  return 0;
}

Bitboard MoveGenerator::knight_attacks(Square square) noexcept {
  // TODO: Prefer a precomputed 64-entry table.
  (void)square;
  return 0;
}

Bitboard MoveGenerator::bishop_attacks(Square square,
                                       Bitboard occupancy) noexcept {
  // TODO: Start with ray stepping; later replace with magic bitboards.
  (void)square;
  (void)occupancy;
  return 0;
}

Bitboard MoveGenerator::rook_attacks(Square square,
                                     Bitboard occupancy) noexcept {
  // TODO: Start with ray stepping; later replace with magic bitboards.
  (void)square;
  (void)occupancy;
  return 0;
}

Bitboard MoveGenerator::queen_attacks(Square square,
                                      Bitboard occupancy) noexcept {
  // TODO: bishop_attacks(square, occupancy) | rook_attacks(square, occupancy).
  (void)square;
  (void)occupancy;
  return 0;
}

Bitboard MoveGenerator::king_attacks(Square square) noexcept {
  // TODO: Prefer a precomputed 64-entry table.
  (void)square;
  return 0;
}

void MoveGenerator::generate_pawns(const Board& board, MoveList& moves,
                                   MoveGenerationType type) noexcept {
  (void)board;
  (void)moves;
  (void)type;
}

void MoveGenerator::generate_knights(const Board& board, MoveList& moves,
                                     MoveGenerationType type) noexcept {
  (void)board;
  (void)moves;
  (void)type;
}

void MoveGenerator::generate_bishops(const Board& board, MoveList& moves,
                                     MoveGenerationType type) noexcept {
  (void)board;
  (void)moves;
  (void)type;
}

void MoveGenerator::generate_rooks(const Board& board, MoveList& moves,
                                   MoveGenerationType type) noexcept {
  (void)board;
  (void)moves;
  (void)type;
}

void MoveGenerator::generate_queens(const Board& board, MoveList& moves,
                                    MoveGenerationType type) noexcept {
  (void)board;
  (void)moves;
  (void)type;
}

void MoveGenerator::generate_king(const Board& board, MoveList& moves,
                                  MoveGenerationType type) noexcept {
  (void)board;
  (void)moves;
  (void)type;
}

bool MoveGenerator::leaves_king_safe(Board& board, Move move) noexcept {
  // TODO: make_move, test in_check(previous side), unmake_move.
  (void)board;
  (void)move;
  return true;
}

} // namespace chesspp::core
