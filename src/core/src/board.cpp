#include "board.hpp"
#include "types.hpp"

#include <array>
#include <bit>
#include <cstdint>
#include <iomanip>
#include <ostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace {

using chesspp::core::COLOR_COUNT;
using chesspp::core::HashKey;
using chesspp::core::PIECE_TYPE_COUNT;
using chesspp::core::Square;
using chesspp::core::SQUARE_COUNT;

inline constexpr std::uint64_t RNG_SEED = 0x9E3779B97F4A7C15ULL;

struct ZobristKeys {
  // random 64-bit key per color/piece/square combination.
  std::array<std::array<std::array<HashKey, SQUARE_COUNT>, PIECE_TYPE_COUNT>,
             COLOR_COUNT>
      pieces{};

  // 16 possible castling states
  // bit 0 white kingside, bit 1 white queenside,
  // bit 2 black kingside, bit 3 black queenside.
  std::array<HashKey, 16> castling{};

  // 8 possible enpassant hashes per file
  std::array<HashKey, 8> en_passant_file{};

  // binary state either white or black to play
  HashKey side_to_move{};
};

ZobristKeys make_zobrist_keys() {
  ZobristKeys keys{};
  std::mt19937_64 rng{RNG_SEED};

  for (auto &color_keys : keys.pieces) {
    for (auto &piece_keys : color_keys) {
      for (HashKey &square_key : piece_keys) {
        square_key = rng();
      }
    }
  }

  for (HashKey &castling_key : keys.castling) {
    castling_key = rng();
  }

  for (HashKey &en_passant_key : keys.en_passant_file) {
    en_passant_key = rng();
  }

  keys.side_to_move = rng();
  return keys;
}

const ZobristKeys &zobrist_keys() {
  static const ZobristKeys keys = make_zobrist_keys();
  return keys;
}

[[nodiscard]] chesspp::core::Bitboard square_mask(Square square) {
  return 1ULL << square;
}

[[nodiscard]] std::size_t
castling_index(const chesspp::core::CastlingRights &rights) {
  return (rights.white_kingside ? 1ULL : 0ULL) |
         (rights.white_queenside ? 2ULL : 0ULL) |
         (rights.black_kingside ? 4ULL : 0ULL) |
         (rights.black_queenside ? 8ULL : 0ULL);
}

[[nodiscard]] std::size_t square_to_file(Square square) {
  return static_cast<std::size_t>(square % 8);
}

[[nodiscard]] const char *color_name(chesspp::core::Color color) {
  return color == chesspp::core::Color::White ? "White" : "Black";
}

[[nodiscard]] const char *piece_name(chesspp::core::PieceType piece) {
  switch (piece) {
  case chesspp::core::PieceType::Pawn:
    return "Pawn";
  case chesspp::core::PieceType::Knight:
    return "Knight";
  case chesspp::core::PieceType::Bishop:
    return "Bishop";
  case chesspp::core::PieceType::Rook:
    return "Rook";
  case chesspp::core::PieceType::Queen:
    return "Queen";
  case chesspp::core::PieceType::King:
    return "King";
  case chesspp::core::PieceType::None:
    return "None";
  }

  return "Unknown";
}

[[nodiscard]] char piece_char(chesspp::core::Color color,
                              chesspp::core::PieceType piece) {
  char symbol = '.';

  switch (piece) {
  case chesspp::core::PieceType::Pawn:
    symbol = 'p';
    break;
  case chesspp::core::PieceType::Knight:
    symbol = 'n';
    break;
  case chesspp::core::PieceType::Bishop:
    symbol = 'b';
    break;
  case chesspp::core::PieceType::Rook:
    symbol = 'r';
    break;
  case chesspp::core::PieceType::Queen:
    symbol = 'q';
    break;
  case chesspp::core::PieceType::King:
    symbol = 'k';
    break;
  case chesspp::core::PieceType::None:
    symbol = '.';
    break;
  }

  if (color == chesspp::core::Color::White && symbol >= 'a' && symbol <= 'z') {
    symbol = static_cast<char>(symbol - ('a' - 'A'));
  }

  return symbol;
}

void write_square(std::ostream &out, Square square) {
  if (square == chesspp::core::NO_SQUARE) {
    out << "-";
    return;
  }

  out << static_cast<char>('a' + (square % 8))
      << static_cast<char>('1' + (square / 8));
}

void write_bitboard(std::ostream &out, chesspp::core::Bitboard bitboard) {
  const auto flags = out.flags();
  const auto fill = out.fill();

  out << "0x" << std::hex << std::uppercase << std::setw(16)
      << std::setfill('0') << bitboard;

  out.flags(flags);
  out.fill(fill);
}

} // namespace

namespace chesspp::core {

Board::Board() { clear(); }

Board Board::starting_position() { return from_fen(STARTING_POSITION_FEN); }

Board Board::from_fen(const char *fen) {
  uint8_t rank = 0;
  uint8_t file = 0;
  Board board;
  for (const char *token = fen; *token != '\0'; token++) {
    if (*token >= '1' && *token <= '8') {
      file += static_cast<uint8_t>(*token - '0');
      continue;
    } else if (*token == '/') {
      rank++;
      file = 0;
      continue;
    } else if (*token == ' ') {
      // TODO: update with e.p. and castling rights parsing
      break;
    }

    const uint8_t board_rank = static_cast<uint8_t>(7 - rank);
    Square square = static_cast<Square>((board_rank * 8) + file);
    std::pair<Color, PieceType> piece_details = char_to_piece(*token);

    if (piece_details.second == PieceType::None) {
      throw std::runtime_error("Error loading from FEN");
    }

    board.set_piece(square, piece_details.first, piece_details.second);
    file++;
  }

  return board;
}

const PieceBitboards &Board::pieces() const noexcept { return pieces_; }

Bitboard Board::pieces(Color color, PieceType piece) const noexcept {
  // TODO: Assert/guard that piece != PieceType::None in debug builds.
  return pieces_[static_cast<std::size_t>(color_index(color))]
                [static_cast<std::size_t>(piece_index(piece))];
}

Bitboard Board::occupancy(Color color) const noexcept {
  return occupancy_by_color_[static_cast<std::size_t>(color_index(color))];
}

Bitboard Board::all_occupancy() const noexcept { return all_occupancy_; }

Color Board::side_to_move() const noexcept { return side_to_move_; }

CastlingRights Board::castling_rights() const noexcept {
  return castling_rights_;
}

Square Board::en_passant_square() const noexcept { return en_passant_square_; }

std::uint16_t Board::halfmove_clock() const noexcept { return halfmove_clock_; }

std::uint16_t Board::fullmove_number() const noexcept {
  return fullmove_number_;
}

HashKey Board::zobrist_key() const noexcept { return zobrist_key_; }

std::optional<PieceType> Board::piece_on(Square square) const noexcept {
  // TODO: Test the square bit against each piece bitboard.
  for (size_t piece = 0; piece < PIECE_TYPE_COUNT; piece++) {
    for (size_t color = 0; color < COLOR_COUNT; color++) {
      if (pieces_[color][piece] & square_mask(square)) {
        return static_cast<PieceType>(piece);
      }
    }
  }
  return std::nullopt;
}

std::optional<Color> Board::color_on(Square square) const noexcept {
  for (size_t piece = 0; piece < PIECE_TYPE_COUNT; piece++) {
    for (size_t color = 0; color < COLOR_COUNT; color++) {
      if (pieces_[color][piece] & square_mask(square)) {
        return static_cast<Color>(color);
      }
    }
  }
  return std::nullopt;
}

std::optional<std::pair<Color, PieceType>>
Board::piece_and_color_on(Square square) const noexcept {
  for (size_t piece = 0; piece < PIECE_TYPE_COUNT; piece++) {
    for (size_t color = 0; color < COLOR_COUNT; color++) {
      if (pieces_[color][piece] & square_mask(square)) {
        return std::make_pair(static_cast<Color>(color),
                              static_cast<PieceType>(piece));
      }
    }
  }
  return std::nullopt;
}

Square Board::king_square(Color color) const noexcept {
  Bitboard kingPos = pieces_[color_index(color)][piece_index(PieceType::King)];
  return static_cast<Square>(std::countr_zero(kingPos));
}

bool Board::in_check(Color color) const noexcept {
  return attackers_to(king_square(color), color);
}

Bitboard Board::attackers_to(Square square, Color by_color) const noexcept {
  // TODO: Combine pawn/knight/slider/king attacks against square.
  (void)square;
  (void)by_color;
  return 0;
}

void Board::clear() noexcept {
  pieces_ = {};
  occupancy_by_color_ = {};
  all_occupancy_ = 0;
  side_to_move_ = Color::White;
  castling_rights_ = {};
  en_passant_square_ = NO_SQUARE;
  halfmove_clock_ = 0;
  fullmove_number_ = 1;
  zobrist_key_ = 0;
}

void Board::set_piece(Square square, Color color, PieceType piece) noexcept {
  const Bitboard mask = square_mask(square);
  pieces_[color_index(color)][piece_index(piece)] |= mask;
  occupancy_by_color_[color_index(color)] |= mask;
  all_occupancy_ |= mask;

  zobrist_key_ ^= zobrist_keys().pieces[color_index(color)][piece_index(piece)]
                                       [static_cast<std::size_t>(square)];
}

void Board::remove_piece(Square square, Color color, PieceType piece) noexcept {
  const Bitboard mask = ~square_mask(square);
  pieces_[color_index(color)][piece_index(piece)] &= mask;
  occupancy_by_color_[color_index(color)] &= mask;
  all_occupancy_ &= mask;

  zobrist_key_ ^= zobrist_keys().pieces[color_index(color)][piece_index(piece)]
                                       [static_cast<std::size_t>(square)];
}

void Board::make_move(Move move, UndoState &undo) noexcept {
  // TODO: Save undo, mutate bitboards/clocks/castling/en-passant/hash.
  // Hash update checklist for real implementation:
  // 1. undo.zobrist_key = zobrist_key_ before changing anything.
  // 2. XOR moving piece off the from-square and on the to-square.
  // 3. If capture, XOR captured piece off its captured square.
  // 4. XOR old castling state off, update rights, XOR new castling state on.
  // 5. XOR old en-passant file off, update square, XOR new file on.
  // 6. XOR zobrist_keys().side_to_move once when side_to_move_ flips.

  (void)move;
  (void)undo;
}

void Board::unmake_move(const UndoState &undo) noexcept {
  // TODO: Restore board from undo and reverse bitboard changes.
  // The simplest and safest hash strategy is to restore:
  // zobrist_key_ = undo.zobrist_key;
  // Then restore pieces/occupancies/clocks/rights to match that position.
  (void)undo;
}

void Board::recompute_derived_state() noexcept {
  occupancy_by_color_ = {};
  all_occupancy_ = 0;
  zobrist_key_ = 0;

  const auto &keys = zobrist_keys();

  for (std::size_t color = 0; color < COLOR_COUNT; ++color) {
    for (std::size_t piece = 0; piece < PIECE_TYPE_COUNT; ++piece) {
      Bitboard bitboard = pieces_[color][piece];
      occupancy_by_color_[color] |= bitboard;

      // TODO: Replace this simple scan with pop_lsb.
      for (std::size_t square = 0; square < SQUARE_COUNT; ++square) {
        if ((bitboard & (1ULL << square)) != 0) {
          zobrist_key_ ^= keys.pieces[color][piece][square];
        }
      }
    }
  }

  all_occupancy_ = occupancy_by_color_[color_index(Color::White)] |
                   occupancy_by_color_[color_index(Color::Black)];

  zobrist_key_ ^= keys.castling[castling_index(castling_rights_)];

  if (en_passant_square_ != NO_SQUARE) {
    zobrist_key_ ^= keys.en_passant_file[square_to_file(en_passant_square_)];
  }

  if (side_to_move_ == Color::Black) {
    zobrist_key_ ^= keys.side_to_move;
  }
}

// verbose flag for extra debug information
void log_board(const Board &board, std::ostream &out, bool verbose) {
  out << "Board\n";
  out << "=====\n\n";

  out << "Position:\n";
  out << "    a b c d e f g h\n";
  out << "  +-----------------+\n";

  for (int rank = 7; rank >= 0; --rank) {
    out << (rank + 1) << " | ";

    for (int file = 0; file < 8; ++file) {
      const Square square = static_cast<Square>((rank * 8) + file);
      const auto piece = board.piece_and_color_on(square);

      if (piece.has_value()) {
        out << piece_char(piece->first, piece->second);
      } else {
        out << '.';
      }

      out << ' ';
    }

    out << "| " << (rank + 1) << '\n';
  }

  out << "  +-----------------+\n";
  out << "    a b c d e f g h\n\n";
  if (verbose) {
    out << "State:\n";
    out << "  Side to move: " << color_name(board.side_to_move()) << '\n';
    out << "  En passant: ";
    write_square(out, board.en_passant_square());
    out << '\n';
    out << "  Halfmove clock: " << board.halfmove_clock() << '\n';
    out << "  Fullmove number: " << board.fullmove_number() << '\n';
    out << "  Zobrist key: ";
    write_bitboard(out, board.zobrist_key());
    out << '\n';

    const CastlingRights rights = board.castling_rights();
    out << "  Castling rights: " << (rights.white_kingside ? 'K' : '-')
        << (rights.white_queenside ? 'Q' : '-')
        << (rights.black_kingside ? 'k' : '-')
        << (rights.black_queenside ? 'q' : '-') << "\n\n";

    out << "Occupancy:\n";
    out << "  White: ";
    write_bitboard(out, board.occupancy(Color::White));
    out << '\n';
    out << "  Black: ";
    write_bitboard(out, board.occupancy(Color::Black));
    out << '\n';
    out << "  All:   ";
    write_bitboard(out, board.all_occupancy());
    out << "\n\n";

    out << "Piece bitboards:\n";
    for (Color color : {Color::White, Color::Black}) {
      out << "  " << color_name(color) << ":\n";

      for (PieceType piece :
           {PieceType::Pawn, PieceType::Knight, PieceType::Bishop,
            PieceType::Rook, PieceType::Queen, PieceType::King}) {
        out << "    " << std::setw(6) << piece_name(piece) << ": ";
        write_bitboard(out, board.pieces(color, piece));
        out << '\n';
      }
    }
  }

  out << '\n';
}

} // namespace chesspp::core
