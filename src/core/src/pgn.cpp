#include "pgn.hpp"

#include <cctype>
#include <cstdlib>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using chesspp::core::Board;
using chesspp::core::Color;
using chesspp::core::Move;
using chesspp::core::MoveFlag;
using chesspp::core::NO_SQUARE;
using chesspp::core::PieceType;
using chesspp::core::Square;

struct SanMove {
  PieceType piece{PieceType::Pawn};
  Square to{NO_SQUARE};
  std::optional<int> from_file{};
  std::optional<int> from_rank{};
  PieceType promotion{PieceType::None};
  bool capture{false};
  bool kingside_castle{false};
  bool queenside_castle{false};
};

[[nodiscard]] int file_of(Square square) {
  return static_cast<int>(square % 8);
}

[[nodiscard]] int rank_of(Square square) {
  return static_cast<int>(square / 8);
}

[[nodiscard]] bool is_file(char token) { return token >= 'a' && token <= 'h'; }

[[nodiscard]] bool is_rank(char token) { return token >= '1' && token <= '8'; }

[[nodiscard]] bool is_result_token(const std::string &token) {
  return token == "1-0" || token == "0-1" || token == "1/2-1/2" || token == "*";
}

[[nodiscard]] MoveFlag add_flag(MoveFlag flags, MoveFlag flag) {
  return static_cast<MoveFlag>(static_cast<std::uint16_t>(flags) |
                               static_cast<std::uint16_t>(flag));
}

[[nodiscard]] PieceType piece_from_san(char token) {
  switch (token) {
  case 'N':
    return PieceType::Knight;
  case 'B':
    return PieceType::Bishop;
  case 'R':
    return PieceType::Rook;
  case 'Q':
    return PieceType::Queen;
  case 'K':
    return PieceType::King;
  default:
    return PieceType::Pawn;
  }
}

// removes metadata and commentary in PGN file
[[nodiscard]] std::string strip_non_moves(const std::string &pgn) {
  std::string moves;
  int variation_depth = 0;
  bool in_header = false;
  bool in_comment = false;
  bool in_semicolon_comment = false;

  for (char token : pgn) {
    if (in_semicolon_comment) {
      if (token == '\n') {
        in_semicolon_comment = false;
        moves.push_back(' ');
      }
      continue;
    }

    if (in_comment) {
      if (token == '}') {
        in_comment = false;
        moves.push_back(' ');
      }
      continue;
    }

    if (in_header) {
      if (token == ']') {
        in_header = false;
        moves.push_back(' ');
      }
      continue;
    }

    if (token == '[') {
      in_header = true;
      continue;
    }

    if (token == '{') {
      in_comment = true;
      continue;
    }

    if (token == ';') {
      in_semicolon_comment = true;
      continue;
    }

    if (token == '(') {
      ++variation_depth;
      continue;
    }

    if (token == ')') {
      if (variation_depth > 0) {
        --variation_depth;
      }
      continue;
    }

    if (variation_depth == 0) {
      moves.push_back(token);
    }
  }

  return moves;
}

[[nodiscard]] bool is_move_number_token(const std::string &token) {
  bool saw_digit = false;
  bool saw_dot = false;

  for (char ch : token) {
    if (std::isdigit(static_cast<unsigned char>(ch)) != 0) {
      saw_digit = true;
      continue;
    }

    if (ch == '.') {
      saw_dot = true;
      continue;
    }

    return false;
  }

  return saw_digit && saw_dot;
}

[[nodiscard]] std::string normalize_token(std::string token) {
  while (!token.empty() && (token.back() == '!' || token.back() == '?' ||
                            token.back() == '+' || token.back() == '#')) {
    token.pop_back();
  }

  return token;
}

[[nodiscard]] std::vector<std::string> move_tokens(const std::string &pgn) {
  const std::string moves = strip_non_moves(pgn);
  std::istringstream stream(moves);
  std::vector<std::string> tokens;
  std::string token;

  while (stream >> token) {
    token = normalize_token(token);
    if (token.empty() || is_result_token(token) ||
        is_move_number_token(token) || token.front() == '$') {
      continue;
    }

    // Handles compact forms such as "1.e4" if they appear.
    const std::size_t dot = token.find_last_of('.');
    if (dot != std::string::npos) {
      token = token.substr(dot + 1);
      token = normalize_token(token);
    }

    if (!token.empty() && !is_result_token(token)) {
      tokens.emplace_back(token);
    }
  }

  return tokens;
}

[[nodiscard]] SanMove parse_san_token(std::string token) {
  SanMove san;

  if (token == "O-O" || token == "0-0") {
    san.kingside_castle = true;
    return san;
  }

  if (token == "O-O-O" || token == "0-0-0") {
    san.queenside_castle = true;
    return san;
  }

  san.capture = token.find('x') != std::string::npos;

  const std::size_t promotion_marker = token.find('=');
  if (promotion_marker != std::string::npos) {
    if (promotion_marker + 1 >= token.size()) {
      throw std::runtime_error("Invalid PGN promotion token: " + token);
    }

    san.promotion = piece_from_san(token[promotion_marker + 1]);
    token = token.substr(0, promotion_marker);
  }

  std::string compact;
  for (char ch : token) {
    if (ch != 'x') {
      compact.push_back(ch);
    }
  }

  if (compact.size() < 2) {
    throw std::runtime_error("Invalid PGN move token: " + token);
  }

  const char to_file = compact[compact.size() - 2];
  const char to_rank = compact[compact.size() - 1];
  if (!is_file(to_file) || !is_rank(to_rank)) {
    throw std::runtime_error("Invalid PGN destination square: " + token);
  }

  san.to = chesspp::core::square_from_algebraic(to_file, to_rank);

  std::string prefix = compact.substr(0, compact.size() - 2);
  if (!prefix.empty()) {
    const PieceType parsed_piece = piece_from_san(prefix.front());
    if (parsed_piece != PieceType::Pawn) {
      san.piece = parsed_piece;
      prefix.erase(prefix.begin());
    }
  }

  for (char ch : prefix) {
    if (is_file(ch)) {
      san.from_file = static_cast<int>(ch - 'a');
    } else if (is_rank(ch)) {
      san.from_rank = static_cast<int>(ch - '1');
    } else {
      throw std::runtime_error("Invalid PGN disambiguation: " + token);
    }
  }

  return san;
}

[[nodiscard]] bool path_clear(const Board &board, Square from, Square to,
                              int file_step, int rank_step) {
  int file = file_of(from) + file_step;
  int rank = rank_of(from) + rank_step;
  const int target_file = file_of(to);
  const int target_rank = rank_of(to);

  while (file != target_file || rank != target_rank) {
    if (board.piece_on(chesspp::core::square_from(file, rank)).has_value()) {
      return false;
    }

    file += file_step;
    rank += rank_step;
  }

  return true;
}

[[nodiscard]] bool piece_can_reach(const Board &board, PieceType piece,
                                   Color color, Square from, Square to,
                                   bool capture) {
  const int from_file = file_of(from);
  const int from_rank = rank_of(from);
  const int to_file = file_of(to);
  const int to_rank = rank_of(to);
  const int file_delta = to_file - from_file;
  const int rank_delta = to_rank - from_rank;
  const int abs_file_delta = std::abs(file_delta);
  const int abs_rank_delta = std::abs(rank_delta);

  switch (piece) {
  case PieceType::Pawn: {
    const int forward = color == Color::White ? 1 : -1;
    const int start_rank = color == Color::White ? 1 : 6;

    if (capture) {
      return abs_file_delta == 1 && rank_delta == forward;
    }

    if (file_delta != 0 || board.piece_on(to).has_value()) {
      return false;
    }

    if (rank_delta == forward) {
      return true;
    }

    if (from_rank == start_rank && rank_delta == 2 * forward) {
      const Square skipped =
          chesspp::core::square_from(from_file, from_rank + forward);
      return !board.piece_on(skipped).has_value();
    }

    return false;
  }
  case PieceType::Knight:
    return (abs_file_delta == 1 && abs_rank_delta == 2) ||
           (abs_file_delta == 2 && abs_rank_delta == 1);
  case PieceType::Bishop:
    return abs_file_delta == abs_rank_delta &&
           path_clear(board, from, to, file_delta > 0 ? 1 : -1,
                      rank_delta > 0 ? 1 : -1);
  case PieceType::Rook:
    if (file_delta == 0 && rank_delta != 0) {
      return path_clear(board, from, to, 0, rank_delta > 0 ? 1 : -1);
    }
    if (rank_delta == 0 && file_delta != 0) {
      return path_clear(board, from, to, file_delta > 0 ? 1 : -1, 0);
    }
    return false;
  case PieceType::Queen:
    if (abs_file_delta == abs_rank_delta) {
      return path_clear(board, from, to, file_delta > 0 ? 1 : -1,
                        rank_delta > 0 ? 1 : -1);
    }
    if (file_delta == 0 && rank_delta != 0) {
      return path_clear(board, from, to, 0, rank_delta > 0 ? 1 : -1);
    }
    if (rank_delta == 0 && file_delta != 0) {
      return path_clear(board, from, to, file_delta > 0 ? 1 : -1, 0);
    }
    return false;
  case PieceType::King:
    return abs_file_delta <= 1 && abs_rank_delta <= 1;
  case PieceType::None:
    return false;
  }

  return false;
}

[[nodiscard]] MoveFlag flags_for_move(const Board &board, const SanMove &san,
                                      Square from, Color color) {
  MoveFlag flags = MoveFlag::Quiet;

  if (san.capture) {
    flags = add_flag(flags, MoveFlag::Capture);
  }

  if (san.piece == PieceType::Pawn &&
      std::abs(rank_of(san.to) - rank_of(from)) == 2) {
    flags = add_flag(flags, MoveFlag::DoublePawnPush);
  }

  if (san.piece == PieceType::Pawn && san.capture &&
      san.to == board.en_passant_square() && !board.piece_on(san.to)) {
    flags = add_flag(flags, MoveFlag::EnPassant);
  }

  if (san.promotion != PieceType::None) {
    flags = add_flag(flags, MoveFlag::Promotion);
  }

  (void)color;
  return flags;
}

[[nodiscard]] Move resolve_san_move(const Board &board, const SanMove &san) {
  const Color color = board.side_to_move();

  if (san.kingside_castle || san.queenside_castle) {
    const char rank = color == Color::White ? '1' : '8';
    const Square from = chesspp::core::square_from_algebraic('e', rank);
    const Square to = chesspp::core::square_from_algebraic(
        san.kingside_castle ? 'g' : 'c', rank);
    return Move(from, to, PieceType::None,
                san.kingside_castle ? MoveFlag::KingCastle
                                    : MoveFlag::QueenCastle);
  }

  std::vector<Square> candidates;
  for (int square = 0; square < chesspp::core::SQUARE_COUNT; ++square) {
    const Square from = static_cast<Square>(square);
    const auto piece_and_color = board.piece_and_color_on(from);
    if (!piece_and_color.has_value() || piece_and_color->first != color ||
        piece_and_color->second != san.piece) {
      continue;
    }

    if (san.from_file.has_value() && file_of(from) != *san.from_file) {
      continue;
    }

    if (san.from_rank.has_value() && rank_of(from) != *san.from_rank) {
      continue;
    }

    const auto target_color = board.color_on(san.to);
    if (target_color.has_value() && *target_color == color) {
      continue;
    }

    if (piece_can_reach(board, san.piece, color, from, san.to, san.capture)) {
      candidates.emplace_back(from);
    }
  }

  if (candidates.empty()) {
    throw std::runtime_error("No source square found for PGN move");
  }

  if (candidates.size() > 1) {
    throw std::runtime_error("Ambiguous PGN move");
  }

  const Square from = candidates.front();
  return Move(from, san.to, san.promotion,
              flags_for_move(board, san, from, color));
}

} // namespace

namespace chesspp::core {

std::vector<Move> parse_pgn_moves(const std::string &pgn) {
  return parse_pgn_moves(pgn, Board::starting_position());
}

std::vector<Move> parse_pgn_moves(const std::string &pgn,
                                  Board initial_position) {
  std::vector<Move> moves;
  Board board = initial_position;

  for (const std::string &token : move_tokens(pgn)) {
    const SanMove san = parse_san_token(token);
    const Move move = resolve_san_move(board, san);
    moves.emplace_back(move);

    UndoState undo;
    board.make_move(move, undo);
  }

  return moves;
}

} // namespace chesspp::core
