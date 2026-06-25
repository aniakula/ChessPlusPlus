#pragma once

#include "board.hpp"

#include <array>
#include <cstddef>

namespace chesspp::core {

inline constexpr std::size_t kMaxMovesPerPosition = 256;

// Stack-friendly move container. Prefer this over allocating std::vector inside
// every alpha-beta node.
class MoveList {
public:
  using Storage = std::array<Move, kMaxMovesPerPosition>;
  using const_iterator = Storage::const_iterator;

  void clear() noexcept;
  void push(Move move) noexcept;

  [[nodiscard]] std::size_t size() const noexcept;
  [[nodiscard]] bool empty() const noexcept;
  [[nodiscard]] const Move& operator[](std::size_t index) const noexcept;
  [[nodiscard]] const_iterator begin() const noexcept;
  [[nodiscard]] const_iterator end() const noexcept;

private:
  Storage moves_{};
  std::size_t size_{0};
};

enum class MoveGenerationType : std::uint8_t {
  All,
  CapturesOnly,
  QuietOnly,
  Evasions,
};

class MoveGenerator {
public:
  MoveGenerator() = delete;

  static void generate_pseudo_legal(const Board& board, MoveList& moves,
                                    MoveGenerationType type =
                                        MoveGenerationType::All) noexcept;
  static void generate_legal(Board& board, MoveList& moves) noexcept;

  // Attack helpers are critical for fast legality checks, check detection, and
  // future quiescence/search extensions.
  [[nodiscard]] static Bitboard pawn_attacks(Color color, Square square) noexcept;
  [[nodiscard]] static Bitboard knight_attacks(Square square) noexcept;
  [[nodiscard]] static Bitboard bishop_attacks(Square square,
                                               Bitboard occupancy) noexcept;
  [[nodiscard]] static Bitboard rook_attacks(Square square,
                                             Bitboard occupancy) noexcept;
  [[nodiscard]] static Bitboard queen_attacks(Square square,
                                              Bitboard occupancy) noexcept;
  [[nodiscard]] static Bitboard king_attacks(Square square) noexcept;

private:
  static void generate_pawns(const Board& board, MoveList& moves,
                             MoveGenerationType type) noexcept;
  static void generate_knights(const Board& board, MoveList& moves,
                               MoveGenerationType type) noexcept;
  static void generate_bishops(const Board& board, MoveList& moves,
                               MoveGenerationType type) noexcept;
  static void generate_rooks(const Board& board, MoveList& moves,
                             MoveGenerationType type) noexcept;
  static void generate_queens(const Board& board, MoveList& moves,
                              MoveGenerationType type) noexcept;
  static void generate_king(const Board& board, MoveList& moves,
                            MoveGenerationType type) noexcept;
  [[nodiscard]] static bool leaves_king_safe(Board& board, Move move) noexcept;
};

}  // namespace chesspp::core
