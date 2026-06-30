#pragma once

#include "game.hpp"
#include "movegen.hpp"
#include "types.hpp"

#include "SFML/System/Vector2.hpp"
#include "SFML/Window/Event.hpp"

#include <optional>

namespace chesspp::app {

struct InputAction {
  enum class Type {
    None,
    SelectSquare,
    PlayMove,
    RequestPromotion,
    Quit,
  };

  Type type{Type::None};
  chesspp::core::Square square{chesspp::core::NO_SQUARE};
  chesspp::core::Move move{};
  chesspp::core::MoveList legal_moves{};
};

class InputHandler {
public:
  InputHandler() = default;

  // Convert SFML input into chess intent. Do not apply moves here; GameLoop
  // should validate through core::Game so input stays thin and testable.
  [[nodiscard]] InputAction transform_event(const sf::Event &event,
                                            const chesspp::core::Game &game,
                                            chesspp::core::Color human_color);

  void clear_selection() noexcept;
  [[nodiscard]] std::optional<chesspp::core::Square>
  selected_square() const noexcept;
  [[nodiscard]] const chesspp::core::MoveList &legal_moves() const noexcept;

private:
  std::optional<chesspp::core::Square> selected_square_{};
  chesspp::core::MoveList legal_moves_{};

  [[nodiscard]] chesspp::core::Square
  pixel_to_square(sf::Vector2i pixel) const noexcept;
  [[nodiscard]] std::optional<chesspp::core::Move>
  find_move_to_square(const chesspp::core::MoveList &legal_moves,
                      chesspp::core::Square to) const noexcept;
};

} // namespace chesspp::app
