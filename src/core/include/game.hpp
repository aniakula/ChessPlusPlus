#pragma once

#include "board.hpp"
#include "movegen.hpp"
#include "types.hpp"

#include <cstddef>
#include <vector>

namespace chesspp::core {

// Game owns the current Board plus rule history that is not part of a single
// position, like repetition. Keep search on Board; keep UI/game flow here.
class Game {
public:
  Game();
  explicit Game(Board board);

  [[nodiscard]] const Board &board() const noexcept;
  [[nodiscard]] Board &board() noexcept;

  [[nodiscard]] MoveList legal_moves() const;
  [[nodiscard]] bool is_legal(Move move) const;
  [[nodiscard]] bool try_make_move(Move move);
  [[nodiscard]] bool is_check() const;
  [[nodiscard]] bool is_checkmate() const;
  [[nodiscard]] bool is_stalemate() const;
  [[nodiscard]] bool is_draw() const;
  [[nodiscard]] DrawReason draw_reason() const;
  [[nodiscard]] GameResult result() const;

  void reset();
  void load_position(Board board);

private:
  Board board_{};
  std::vector<HashKey> repetition_history_{};
  void record_repetition_key();
  [[nodiscard]] std::size_t repetition_count(HashKey key) const;
  [[nodiscard]] bool has_insufficient_material() const;
};

} // namespace chesspp::core
