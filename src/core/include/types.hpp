#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <ostream>
#include <utility>

namespace chesspp::core {

using Bitboard = std::uint64_t;
using HashKey = std::uint64_t;
using Square = std::uint8_t;
using Score = std::int32_t;
using Phase = std::uint8_t;

// score of something at start of game vs end of game:
using Weight = std::pair<Score, Score>;

// for less wide Position score arrays:
using W = Weight;

inline constexpr Square NO_SQUARE = 64;
inline constexpr int SQUARE_COUNT = 64;
inline constexpr int COLOR_COUNT = 2;
inline constexpr int PIECE_TYPE_COUNT = 6;
inline constexpr const char *STARTING_POSITION_FEN =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// phase weights for interpolation:
inline constexpr Phase OPENING_PHASE_WEIGHT = 24;
inline constexpr Phase KNIGHTS_PHASE_WEIGHT = 1;
inline constexpr Phase BISHOPS_PHASE_WEIGHT = 1;
inline constexpr Phase ROOKS_PHASE_WEIGHT = 2;
inline constexpr Phase QUEENS_PHASE_WEIGHT = 4;

// all scores are in centi-pawns (1/100th of a pawn's value)
inline constexpr Score INF = std::numeric_limits<Score>::max() / 2;
inline constexpr Score MATE_SCORE = INF;

inline constexpr Score PAWN_SCORE = 100;
inline constexpr Score KNIGHT_SCORE = 320;
inline constexpr Score BISHOP_SCORE = 330;
inline constexpr Score ROOK_SCORE = 500;
inline constexpr Score QUEEN_SCORE = 900;

inline constexpr std::array<Weight, 64> PAWN_POSITION_SCORE{};
inline constexpr std::array<Weight, 64> KNIGHT_POSITION_SCORE{};
inline constexpr std::array<Weight, 64> BISHOP_POSITION_SCORE{};
inline constexpr std::array<Weight, 64> ROOK_POSITION_SCORE{};
inline constexpr std::array<Weight, 64> QUEEN_POSITION_SCORE{};
inline constexpr std::array<Weight, 64> KING_POSITION_SCORE{};

inline constexpr Weight BISHOP_PAIR_WEIGHT{10, 40};

enum class Color : std::uint8_t {
  White = 0,
  Black = 1,
};

enum class PieceType : std::uint8_t {
  Pawn = 0,
  Knight = 1,
  Bishop = 2,
  Rook = 3,
  Queen = 4,
  King = 5,
  None = 6,
};

[[nodiscard]] constexpr Phase phase_weight(PieceType piece) noexcept {
  switch (piece) {
  case PieceType::Knight:
    return KNIGHTS_PHASE_WEIGHT;
  case PieceType::Bishop:
    return BISHOPS_PHASE_WEIGHT;
  case PieceType::Rook:
    return ROOKS_PHASE_WEIGHT;
  case PieceType::Queen:
    return QUEENS_PHASE_WEIGHT;
  default:
    return 0;
  }
}

// Interpolate opening/endgame scores by remaining game phase.
[[nodiscard]] constexpr Score taper(Weight weight, Phase phase) noexcept {
  return (weight.first * static_cast<Score>(phase) +
          weight.second * static_cast<Score>(OPENING_PHASE_WEIGHT - phase)) /
         static_cast<Score>(OPENING_PHASE_WEIGHT);
}

[[nodiscard]] constexpr Phase clamp_phase(int phase) noexcept {
  if (phase < 0) {
    return 0;
  }
  if (phase > static_cast<int>(OPENING_PHASE_WEIGHT)) {
    return OPENING_PHASE_WEIGHT;
  }
  return static_cast<Phase>(phase);
}

enum class GameResult : std::uint8_t {
  Ongoing,
  WhiteWins,
  BlackWins,
  Draw,
};

enum class DrawReason : std::uint8_t {
  None,
  Stalemate,
  FiftyMoveRule,
  ThreefoldRepetition,
  InsufficientMaterial,
};

enum class MoveFlag : std::uint16_t {
  Quiet = 0,
  Capture = 1 << 0,
  DoublePawnPush = 1 << 1,
  EnPassant = 1 << 2,
  KingCastle = 1 << 3,
  QueenCastle = 1 << 4,
  Promotion = 1 << 5,
};

[[nodiscard]] constexpr char piece_to_char(Color color, PieceType piece) {
  char diff = color == Color::Black ? 0 : 56;
  switch (piece) {
  case PieceType::Pawn:
    return ('p' - diff);
  case PieceType::Knight:
    return ('n' - diff);
  case PieceType::Bishop:
    return ('b' - diff);
  case PieceType::Rook:
    return ('r' - diff);
  case PieceType::Queen:
    return ('q' - diff);
  case PieceType::King:
    return ('k' - diff);
  default:
    return '?';
  }
}

[[nodiscard]] constexpr std::pair<Color, PieceType> char_to_piece(char token) {
  switch (token) {
  case 'p':
    return std::make_pair(Color::Black, PieceType::Pawn);
  case 'n':
    return std::make_pair(Color::Black, PieceType::Knight);
  case 'b':
    return std::make_pair(Color::Black, PieceType::Bishop);
  case 'r':
    return std::make_pair(Color::Black, PieceType::Rook);
  case 'q':
    return std::make_pair(Color::Black, PieceType::Queen);
  case 'k':
    return std::make_pair(Color::Black, PieceType::King);
  case 'P':
    return std::make_pair(Color::White, PieceType::Pawn);
  case 'N':
    return std::make_pair(Color::White, PieceType::Knight);
  case 'B':
    return std::make_pair(Color::White, PieceType::Bishop);
  case 'R':
    return std::make_pair(Color::White, PieceType::Rook);
  case 'Q':
    return std::make_pair(Color::White, PieceType::Queen);
  case 'K':
    return std::make_pair(Color::White, PieceType::King);
  default:
    return std::make_pair(Color::White, PieceType::None);
  }
}

[[nodiscard]] constexpr std::size_t color_index(Color color) {
  return static_cast<std::size_t>(color);
}

[[nodiscard]] constexpr std::size_t piece_index(PieceType piece) {
  return static_cast<std::size_t>(piece);
}

[[nodiscard]] constexpr Color opposite(Color color) {
  return color == Color::White ? Color::Black : Color::White;
}

[[nodiscard]] constexpr Square square_from(int file, int rank) {
  return static_cast<Square>((rank * 8) + file);
}

[[nodiscard]] constexpr Square square_from_algebraic(char file, char rank) {
  return square_from(static_cast<int>(file - 'a'),
                     static_cast<int>(rank - '1'));
}

[[nodiscard]] constexpr MoveFlag operator|(MoveFlag lhs, MoveFlag rhs) {
  return static_cast<MoveFlag>(static_cast<std::uint16_t>(lhs) |
                               static_cast<std::uint16_t>(rhs));
}

[[nodiscard]] constexpr bool has_flag(MoveFlag flags, MoveFlag flag) {
  return (static_cast<std::uint16_t>(flags) &
          static_cast<std::uint16_t>(flag)) != 0;
}

[[nodiscard]] constexpr Square pop_lsb(Bitboard &bitboard) noexcept {
  const Square square = static_cast<Square>(std::countr_zero(bitboard));
  bitboard &= bitboard - 1;
  return square;
}

// move encoding Layout:
// bits 0-5   from square
// bits 6-11  to square
// bits 12-15 promotion piece
// bits 16-31 flags
class Move {
public:
  constexpr Move() = default;
  constexpr Move(Square from, Square to, PieceType promotion = PieceType::None,
                 MoveFlag flags = MoveFlag::Quiet)
      : data_{static_cast<std::uint32_t>(from) |
              (static_cast<std::uint32_t>(to) << 6) |
              (static_cast<std::uint32_t>(promotion) << 12) |
              (static_cast<std::uint32_t>(flags) << 16)} {}

  [[nodiscard]] static constexpr Move null() { return Move{}; }
  [[nodiscard]] constexpr bool is_null() const { return data_ == 0; }
  [[nodiscard]] constexpr Square from() const {
    return static_cast<Square>(data_ & 0x3F);
  }
  [[nodiscard]] constexpr Square to() const {
    return static_cast<Square>((data_ >> 6) & 0x3F);
  }
  [[nodiscard]] constexpr PieceType promotion() const {
    return static_cast<PieceType>((data_ >> 12) & 0x0F);
  }
  [[nodiscard]] constexpr MoveFlag flags() const {
    return static_cast<MoveFlag>((data_ >> 16) & 0xFFFF);
  }
  [[nodiscard]] constexpr std::uint32_t raw() const { return data_; }

  [[nodiscard]] constexpr bool operator==(const Move &) const = default;

private:
  std::uint32_t data_{0};
};

inline std::ostream &operator<<(std::ostream &out, Move move) {
  const auto write_square = [&out](Square square) {
    if (square == NO_SQUARE) {
      out << "-";
      return;
    }

    out << static_cast<char>('a' + (square % 8))
        << static_cast<char>('1' + (square / 8));
  };

  if (move.is_null()) {
    out << "<null move>";
    return out;
  }

  write_square(move.from());
  write_square(move.to());

  if (move.promotion() != PieceType::None) {
    switch (move.promotion()) {
    case PieceType::Knight:
      out << "=N";
      break;
    case PieceType::Bishop:
      out << "=B";
      break;
    case PieceType::Rook:
      out << "=R";
      break;
    case PieceType::Queen:
      out << "=Q";
      break;
    default:
      break;
    }
  }

  const MoveFlag flags = move.flags();
  if (flags == MoveFlag::Quiet) {
    return out;
  }

  const char *separator = " (";
  const auto write_flag = [&](MoveFlag flag, const char *name) {
    if (has_flag(flags, flag)) {
      out << separator << name;
      separator = ", ";
    }
  };

  write_flag(MoveFlag::Capture, "capture");
  write_flag(MoveFlag::DoublePawnPush, "double-pawn-push");
  write_flag(MoveFlag::EnPassant, "en-passant");
  write_flag(MoveFlag::KingCastle, "king-castle");
  write_flag(MoveFlag::QueenCastle, "queen-castle");
  write_flag(MoveFlag::Promotion, "promotion");

  out << ')';
  return out;
}

struct ScoredMove {
  Move move{};
  Score score{0};
};

// bitboard per piece per color:
using PieceBitboards =
    std::array<std::array<Bitboard, PIECE_TYPE_COUNT>, COLOR_COUNT>;

} // namespace chesspp::core
