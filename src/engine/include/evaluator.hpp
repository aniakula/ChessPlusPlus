#pragma once

#include "board.hpp"
#include "types.hpp"

namespace chesspp::engine {

struct EvaluationConfig {
  bool use_piece_square_tables{true};
  bool use_mobility{false};
  bool use_king_safety{false};
};

class Evaluator {
public:
  explicit Evaluator(EvaluationConfig config = {});

  // Score from White's point of view. Search can multiply/negate based on side
  // to move. Keep this allocation-free and branch-light.
  [[nodiscard]] chesspp::core::Score
  evaluate(const chesspp::core::Board& board) const noexcept;

  [[nodiscard]] chesspp::core::Score
  material_score(const chesspp::core::Board& board) const noexcept;

  [[nodiscard]] chesspp::core::Score
  piece_square_score(const chesspp::core::Board& board) const noexcept;

private:
  EvaluationConfig config_{};
};

}  // namespace chesspp::engine
