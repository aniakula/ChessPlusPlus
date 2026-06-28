#include "board.hpp"
#include "movegen.hpp"
#include "types.hpp"

#include <array>
#include <bit>
#include <cstdint>
#include <exception>
#include <iomanip>
#include <ostream>
#include <random>
#include <stdexcept>
#include <string>
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

const ZobristKeys &zobrist_hashes() {
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
void update_castling_rights(chesspp::core::CastlingRights &rights,
                            const chesspp::core::PieceType &moved,
                            const chesspp::core::MoveFlag &flags,
                            const chesspp::core::Color &us,
                            const chesspp::core::Square &from,
                            const chesspp::core::Square &to) {
  if (chesspp::core::has_flag(flags, chesspp::core::MoveFlag::KingCastle) ||
      chesspp::core::has_flag(flags, chesspp::core::MoveFlag::QueenCastle) ||
      moved == chesspp::core::PieceType::King) {
    chesspp::core::Board::disable_castling_rights(us, rights);
  } else if (moved == chesspp::core::PieceType::Rook &&
             (from == 0 || from == 56)) {
    chesspp::core::Board::disable_queen_side_castling_rights(us, rights);
  } else if (moved == chesspp::core::PieceType::Rook &&
             (from == 7 || from == 63)) {
    chesspp::core::Board::disable_king_side_castling_rights(us, rights);
  }

  if (to == 0) {
    chesspp::core::Board::disable_queen_side_castling_rights(
        chesspp::core::Color::White, rights);
    return;
  } else if (to == 56) {
    chesspp::core::Board::disable_queen_side_castling_rights(
        chesspp::core::Color::Black, rights);
    return;
  } else if (to == 7) {
    chesspp::core::Board::disable_king_side_castling_rights(
        chesspp::core::Color::White, rights);
    return;
  } else if (to == 63) {
    chesspp::core::Board::disable_king_side_castling_rights(
        chesspp::core::Color::Black, rights);
  }
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
      token++;
      try {
        parse_fen_closer(token, board);
      } catch (std::exception e) {
        throw std::runtime_error("Error Loading FEN Trailing String");
      }

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

void Board::parse_fen_closer(const char *fen, Board &board) {
  std::string closer(fen);
  std::array<std::string, 5> parsed;
  size_t start = 0;
  size_t paramInd = 0;
  for (size_t i = 0; i < closer.size(); i++) {
    if (closer[i] == ' ') {
      parsed[paramInd] = closer.substr(start, (i - start));
      start = i + 1;
      paramInd++;
    }
  }
  parsed[paramInd] = closer.substr(start, closer.size() - start);

  if (parsed[0] == "w") {
    board.set_side_to_move(Color::White);
  } else {
    board.set_side_to_move(Color::Black);
    board.hash(zobrist_hashes().side_to_move);
  }

  board.disable_castling_rights();
  if (parsed[1] != "-") {
    for (char castle : parsed[1]) {
      switch (castle) {
      case 'k':
        board.set_castling_rights(Color::Black, PieceType::King, true);
        break;
      case 'K':
        board.set_castling_rights(Color::White, PieceType::King, true);
        break;
      case 'q':
        board.set_castling_rights(Color::Black, PieceType::Queen, true);
        break;
      case 'Q':
        board.set_castling_rights(Color::White, PieceType::Queen, true);
        break;
      }
    }
  }
  size_t castling_hash_index = castling_index(board.castling_rights());
  board.hash(zobrist_hashes().castling[castling_hash_index]);

  if (parsed[2] != "-") {
    board.set_ep_square(square_from_algebraic(parsed[2][0], parsed[2][1]));
    size_t ep_hash_index = square_to_file(board.en_passant_square());
    board.hash(zobrist_hashes().en_passant_file[ep_hash_index]);
  } else {
    board.set_ep_square(NO_SQUARE);
  }

  board.set_half_moves(static_cast<uint16_t>(stoi(parsed[3])));
  board.set_full_moves(static_cast<uint16_t>(stoi(parsed[4])));
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
  return attackers_to(king_square(color), opposite(color)) != 0;
}

Bitboard Board::attackers_to(Square square, Color by_color) const noexcept {
  const Bitboard occ = all_occupancy_;
  const Bitboard pawns =
      pieces_[color_index(by_color)][piece_index(PieceType::Pawn)];
  const Bitboard knights =
      pieces_[color_index(by_color)][piece_index(PieceType::Knight)];
  const Bitboard bishops =
      pieces_[color_index(by_color)][piece_index(PieceType::Bishop)];
  const Bitboard rooks =
      pieces_[color_index(by_color)][piece_index(PieceType::Rook)];
  const Bitboard queens =
      pieces_[color_index(by_color)][piece_index(PieceType::Queen)];
  const Bitboard king =
      pieces_[color_index(by_color)][piece_index(PieceType::King)];
  Bitboard attackers = 0;
  //(pawns attacking a square can only be on the attacked squares of a pawn of
  // opposite color)
  attackers |= MoveGenerator::pawn_attacks(opposite(by_color), square) & pawns;
  attackers |= MoveGenerator::knight_attacks(square) & knights;
  attackers |= MoveGenerator::bishop_attacks(square, occ) & (bishops | queens);
  attackers |= MoveGenerator::rook_attacks(square, occ) & (rooks | queens);
  attackers |= MoveGenerator::king_attacks(square) & king;
  return attackers;
}

// setters:

void Board::set_side_to_move(Color color) { side_to_move_ = color; }

void Board::set_castling_rights(Color color, PieceType side, bool set) {
  if (color == Color::Black) {
    if (side == PieceType::Queen) {
      castling_rights_.black_queenside = set;
    } else {
      castling_rights_.black_kingside = set;
    }
  } else {
    if (side == PieceType::Queen) {
      castling_rights_.white_queenside = set;
    } else {
      castling_rights_.white_kingside = set;
    }
  }
}

void Board::disable_castling_rights() {
  set_castling_rights(Color::Black, PieceType::King, false);
  set_castling_rights(Color::White, PieceType::King, false);
  set_castling_rights(Color::Black, PieceType::Queen, false);
  set_castling_rights(Color::White, PieceType::Queen, false);
}

void Board::disable_castling_rights(const Color &color) {
  if (color == Color::White) {
    set_castling_rights(Color::White, PieceType::King, false);
    set_castling_rights(Color::White, PieceType::Queen, false);
  } else {
    set_castling_rights(Color::Black, PieceType::King, false);
    set_castling_rights(Color::Black, PieceType::Queen, false);
  }
}

void Board::disable_castling_rights(const Color &color,
                                    CastlingRights &rights) {
  if (color == Color::White) {
    rights.white_queenside = false;
    rights.white_kingside = false;
  } else {
    rights.black_queenside = false;
    rights.black_kingside = false;
  }
}

void Board::disable_queen_side_castling_rights(const Color &color,
                                               CastlingRights &rights) {
  if (color == Color::White) {
    rights.white_queenside = false;
  } else {
    rights.black_queenside = false;
  }
}

void Board::disable_king_side_castling_rights(const Color &color,
                                              CastlingRights &rights) {
  if (color == Color::White) {
    rights.white_kingside = false;
  } else {
    rights.black_kingside = false;
  }
}

void Board::set_ep_square(Square square) { en_passant_square_ = square; }

void Board::set_full_moves(uint16_t moves) { fullmove_number_ = moves; }

void Board::set_half_moves(uint16_t moves) { halfmove_clock_ = moves; }

void Board::hash(HashKey key) { zobrist_key_ ^= key; }

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

  zobrist_key_ ^=
      zobrist_hashes().pieces[color_index(color)][piece_index(piece)]
                             [static_cast<std::size_t>(square)];
}

void Board::remove_piece(Square square, Color color, PieceType piece) noexcept {
  const Bitboard mask = ~square_mask(square);
  pieces_[color_index(color)][piece_index(piece)] &= mask;
  occupancy_by_color_[color_index(color)] &= mask;
  all_occupancy_ &= mask;

  zobrist_key_ ^=
      zobrist_hashes().pieces[color_index(color)][piece_index(piece)]
                             [static_cast<std::size_t>(square)];
}

void Board::make_move(Move move, UndoState &undo) noexcept {
  const Square from = move.from();
  const Square to = move.to();
  const MoveFlag flags = move.flags();
  const Color us = side_to_move_;
  const Color them = opposite(us);

  undo.move = move;
  undo.zobrist_key = zobrist_key_;
  undo.castling_rights = castling_rights_;
  undo.en_passant_square = en_passant_square_;
  undo.halfmove_clock = halfmove_clock_;
  undo.fullmove_number = fullmove_number_;
  undo.captured_piece = PieceType::None;
  undo.captured_square = NO_SQUARE;

  const PieceType moving = piece_on(from).value_or(PieceType::None);
  undo.moved_piece = moving;

  if (has_flag(flags, MoveFlag::EnPassant)) {
    undo.captured_square =
        static_cast<Square>(us == Color::White ? to - 8 : to + 8);
    undo.captured_piece = PieceType::Pawn;
    remove_piece(undo.captured_square, them, PieceType::Pawn);
  } else if (has_flag(flags, MoveFlag::Capture)) {
    undo.captured_square = to;
    undo.captured_piece = piece_on(to).value_or(PieceType::None);
    remove_piece(to, them, undo.captured_piece);
  }

  remove_piece(from, us, moving);
  const PieceType placed =
      has_flag(flags, MoveFlag::Promotion) ? move.promotion() : moving;
  set_piece(to, us, placed);

  if (has_flag(flags, MoveFlag::KingCastle)) {
    remove_piece(to + 1, us, PieceType::Rook);
    set_piece(to - 1, us, PieceType::Rook);
  } else if (has_flag(flags, MoveFlag::QueenCastle)) {
    remove_piece(to - 2, us, PieceType::Rook);
    set_piece(to + 1, us, PieceType::Rook);
  }

  const auto &keys = zobrist_hashes();
  zobrist_key_ ^= keys.castling[castling_index(castling_rights_)];
  update_castling_rights(castling_rights_, placed, flags, us, from, to);
  zobrist_key_ ^= keys.castling[castling_index(castling_rights_)];

  if (en_passant_square_ != NO_SQUARE) {
    zobrist_key_ ^= keys.en_passant_file[square_to_file(en_passant_square_)];
  }

  en_passant_square_ = NO_SQUARE;
  if (has_flag(flags, MoveFlag::DoublePawnPush)) {
    en_passant_square_ =
        static_cast<Square>(us == Color::White ? to - 8 : to + 8);
    zobrist_key_ ^= keys.en_passant_file[square_to_file(en_passant_square_)];
  }

  if (moving == PieceType::Pawn || has_flag(flags, MoveFlag::Capture)) {
    halfmove_clock_ = 0;
  } else {
    ++halfmove_clock_;
  }

  if (us == Color::Black) {
    ++fullmove_number_;
  }

  zobrist_key_ ^= keys.side_to_move;
  side_to_move_ = them;
}

void Board::unmake_move(const UndoState &undo) noexcept {
  const Move move = undo.move;
  const Square from = move.from();
  const Square to = move.to();
  const MoveFlag flags = move.flags();
  const Color us = opposite(side_to_move_);

  // set rook position based on king's landing spot:
  if (has_flag(flags, MoveFlag::KingCastle)) {
    remove_piece(to - 1, us, PieceType::Rook);
    set_piece(to + 1, us, PieceType::Rook);
  } else if (has_flag(flags, MoveFlag::QueenCastle)) {
    remove_piece(to + 1, us, PieceType::Rook);
    set_piece(to - 2, us, PieceType::Rook);
  }

  const PieceType placed = has_flag(flags, MoveFlag::Promotion)
                               ? move.promotion()
                               : undo.moved_piece;
  // set moved piece to original location:
  remove_piece(to, us, placed);
  set_piece(from, us, undo.moved_piece);

  // reset captured piece:
  if (undo.captured_piece != PieceType::None) {
    set_piece(undo.captured_square, opposite(us), undo.captured_piece);
  }

  side_to_move_ = us;
  castling_rights_ = undo.castling_rights;
  en_passant_square_ = undo.en_passant_square;
  halfmove_clock_ = undo.halfmove_clock;
  fullmove_number_ = undo.fullmove_number;
  zobrist_key_ = undo.zobrist_key;
}

void Board::recompute_derived_state() noexcept {
  occupancy_by_color_ = {};
  all_occupancy_ = 0;
  zobrist_key_ = 0;

  const auto &keys = zobrist_hashes();

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
