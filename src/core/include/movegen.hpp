#pragma once

#include "board.hpp"
#include "types.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

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
  [[nodiscard]] const Move &operator[](std::size_t index) const noexcept;
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

namespace MoveGenerator {

void generate_pseudo_legal(
    const Board &board, MoveList &moves,
    MoveGenerationType type = MoveGenerationType::All) noexcept;
void generate_legal(Board &board, MoveList &moves) noexcept;

// Attack helpers are critical for fast legality checks, check detection, and
// future quiescence/search extensions.
[[nodiscard]] Bitboard pawn_attacks(Color color, Square square) noexcept;
[[nodiscard]] Bitboard knight_attacks(Square square) noexcept;
[[nodiscard]] Bitboard bishop_attacks(Square square,
                                      Bitboard occupancy) noexcept;
[[nodiscard]] Bitboard rook_attacks(Square square, Bitboard occupancy) noexcept;
[[nodiscard]] Bitboard queen_attacks(Square square,
                                     Bitboard occupancy) noexcept;
[[nodiscard]] Bitboard king_attacks(Square square) noexcept;

void generate_pawns(const Board &board, MoveList &moves,
                    MoveGenerationType type) noexcept;
void generate_knights(const Board &board, MoveList &moves,
                      MoveGenerationType type) noexcept;
void generate_bishops(const Board &board, MoveList &moves,
                      MoveGenerationType type) noexcept;
void generate_rooks(const Board &board, MoveList &moves,
                    MoveGenerationType type) noexcept;
void generate_queens(const Board &board, MoveList &moves,
                     MoveGenerationType type) noexcept;
void generate_king(const Board &board, MoveList &moves,
                   MoveGenerationType type) noexcept;
[[nodiscard]] bool leaves_king_safe(Board &board, Move move) noexcept;

inline constexpr std::array<std::array<int8_t, 2>, 8> knightDeltas = {{
    {2, -1},
    {2, 1},
    {-2, -1},
    {-2, 1},
    {1, -2},
    {1, 2},
    {-1, -2},
    {-1, 2},
}};

inline constexpr std::array<std::array<int8_t, 2>, 8> kingDeltas = {{
    {1, -1},
    {1, 1},
    {-1, 0},
    {-1, -1},
    {-1, 1},
    {-1, 0},
    {0, -1},
    {0, 1},
}};

inline constexpr const std::array<Bitboard, 64> king_attack_table = []() {
  std::array<Bitboard, 64> attacks;
  for (int i = 0; i < 64; i++) {
    Bitboard currAttacks{0};
    int row = i / 8;
    int col = i % 8;
    for (auto delta : kingDeltas) {
      Bitboard currDelta{0};
      if (row + delta[0] >= 0 && row + delta[0] < 8 && col + delta[1] >= 0 &&
          col + delta[1] < 8) {
        currDelta = Bitboard{1} << ((row + delta[0]) * 8 + col + delta[1]);
      }
      currAttacks |= currDelta;
    }
    attacks[static_cast<size_t>(i)] = currAttacks;
  }

  return attacks;
}();

inline constexpr const std::array<Bitboard, 64> knight_attack_table = []() {
  std::array<Bitboard, 64> attacks;
  for (int i = 0; i < 64; i++) {
    Bitboard currAttacks{0};
    int row = i / 8;
    int col = i % 8;
    for (auto delta : knightDeltas) {
      Bitboard currDelta{0};
      if (row + delta[0] >= 0 && row + delta[0] < 8 && col + delta[1] >= 0 &&
          col + delta[1] < 8) {
        currDelta = Bitboard{1} << ((row + delta[0]) * 8 + col + delta[1]);
      }
      currAttacks |= currDelta;
    }
    attacks[static_cast<size_t>(i)] = currAttacks;
  }

  return attacks;
}();
}; // namespace MoveGenerator

} // namespace chesspp::core
