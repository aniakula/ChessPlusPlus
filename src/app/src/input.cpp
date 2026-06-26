#include "input.hpp"

#include "SFML/Window/Mouse.hpp"

namespace chesspp::app {

namespace {

constexpr int BOARD_SIZE = 800;
constexpr int TILE_SIZE = BOARD_SIZE / 8;

} // namespace

InputAction InputHandler::transform_event(const sf::Event &event,
                                          const chesspp::core::Game &game) {
  if (const auto *mouse_press = event.getIf<sf::Event::MouseButtonPressed>()) {
    if (mouse_press->button != sf::Mouse::Button::Left) {
      return {};
    }

    const chesspp::core::Square clicked =
        pixel_to_square(mouse_press->position);
    if (clicked == chesspp::core::NO_SQUARE) {
      clear_selection();
      return {};
    }

    selected_square_ = clicked;

    // Temporary behavior until legal move generation/input move selection is
    // complete: every click selects and highlights exactly that square.
    const auto piece = game.board().piece_and_color_on(clicked);
    (void)piece;

    InputAction action{};
    action.type = InputAction::Type::SelectSquare;
    action.square = clicked;
    return action;
  }

  return {};
}

void InputHandler::clear_selection() noexcept { selected_square_.reset(); }

std::optional<chesspp::core::Square>
InputHandler::selected_square() const noexcept {
  return selected_square_;
}

chesspp::core::Square
InputHandler::pixel_to_square(sf::Vector2i pixel) const noexcept {
  if (pixel.x < 0 || pixel.y < 0 || pixel.x >= BOARD_SIZE ||
      pixel.y >= BOARD_SIZE) {
    return chesspp::core::NO_SQUARE;
  }

  const int file = pixel.x / TILE_SIZE;
  const int screen_rank = pixel.y / TILE_SIZE;
  const int rank = 7 - screen_rank;

  return chesspp::core::square_from(file, rank);
}

std::optional<chesspp::core::Move>
InputHandler::find_move_to_square(const chesspp::core::MoveList &legal_moves,
                                  chesspp::core::Square to) const noexcept {
  if (!selected_square_.has_value()) {
    return std::nullopt;
  }

  for (const chesspp::core::Move move : legal_moves) {
    if (move.from() == *selected_square_ && move.to() == to) {
      return move;
    }
  }

  return std::nullopt;
}

} // namespace chesspp::app
