#pragma once

#include "types.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <ostream>
#include <utility>

namespace chesspp::core {

struct CastlingRights {
  bool white_kingside{true};
  bool white_queenside{true};
  bool black_kingside{true};
  bool black_queenside{true};
};

// Save the state needed to reverse a move
struct UndoState {
  Move move{};
  PieceType moved_piece{PieceType::None};
  PieceType captured_piece{PieceType::None};
  Square captured_square{NO_SQUARE};
  CastlingRights castling_rights{};
  Square en_passant_square{NO_SQUARE};
  std::uint16_t halfmove_clock{0};
  std::uint16_t fullmove_number{1};
  HashKey zobrist_key{0};
};

class Board {
public:
  Board();

  [[nodiscard]] static Board starting_position();
  [[nodiscard]] static Board from_fen(const char *fen);
  static void parse_fen_closer(const char *fen, Board &board);

  [[nodiscard]] const PieceBitboards &pieces() const noexcept;
  [[nodiscard]] Bitboard pieces(Color color, PieceType piece) const noexcept;
  [[nodiscard]] Bitboard occupancy(Color color) const noexcept;
  [[nodiscard]] Bitboard all_occupancy() const noexcept;

  [[nodiscard]] Color side_to_move() const noexcept;
  [[nodiscard]] CastlingRights castling_rights() const noexcept;
  [[nodiscard]] Square en_passant_square() const noexcept;
  [[nodiscard]] std::uint16_t halfmove_clock() const noexcept;
  [[nodiscard]] std::uint16_t fullmove_number() const noexcept;
  [[nodiscard]] HashKey zobrist_key() const noexcept;

  void set_side_to_move(Color color);
  void set_castling_rights(Color color, PieceType side, bool set);
  void disable_castling_rights();
  void disable_castling_rights(const Color &color);
  static void disable_castling_rights(const Color &color,
                                      CastlingRights &rights);
  static void disable_queen_side_castling_rights(const Color &color,
                                                 CastlingRights &rights);
  static void disable_king_side_castling_rights(const Color &color,
                                                CastlingRights &rights);
  void set_ep_square(Square square);
  void set_full_moves(uint16_t moves);
  void set_half_moves(uint16_t moves);

  void hash(HashKey key);

  [[nodiscard]] std::optional<PieceType> piece_on(Square square) const noexcept;
  [[nodiscard]] std::optional<Color> color_on(Square square) const noexcept;
  [[nodiscard]] std::optional<std::pair<Color, PieceType>>
  piece_and_color_on(Square square) const noexcept;

  [[nodiscard]] Square king_square(Color color) const noexcept;
  [[nodiscard]] bool in_check(Color color) const noexcept;
  [[nodiscard]] Bitboard attackers_to(Square square,
                                      Color by_color) const noexcept;

  void clear() noexcept;
  void set_piece(Square square, Color color, PieceType piece) noexcept;
  void remove_piece(Square square, Color color, PieceType piece) noexcept;
  void make_move(Move move, UndoState &undo) noexcept;
  void unmake_move(const UndoState &undo) noexcept;

  // Used for loading FEN/tests/debugging
  void recompute_derived_state() noexcept;

private:
  PieceBitboards pieces_{};
  std::array<Bitboard, COLOR_COUNT> occupancy_by_color_{};
  Bitboard all_occupancy_{0};
  Color side_to_move_{Color::White};
  CastlingRights castling_rights_{};
  Square en_passant_square_{NO_SQUARE};
  std::uint16_t halfmove_clock_{0};
  std::uint16_t fullmove_number_{1};
  HashKey zobrist_key_{0};
};

// verbose flag for extra debug information
void log_board(const Board &board, std::ostream &out, bool verbose = false);

} // namespace chesspp::core
