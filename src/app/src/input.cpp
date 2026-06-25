#include "input.hpp"

namespace chesspp::app {

InputAction InputHandler::handle_event(const sf::Event& event,
                                       const chesspp::core::Game& game) {
  // TODO: Convert SFML mouse/keyboard events into InputAction.
  (void)event;
  (void)game;
  return {};
}

void InputHandler::clear_selection() noexcept { selected_square_.reset(); }

std::optional<chesspp::core::Square> InputHandler::selected_square()
    const noexcept {
  return selected_square_;
}

chesspp::core::Square InputHandler::pixel_to_square(
    sf::Vector2i pixel) const noexcept {
  // TODO: Keep this mapping consistent with Renderer::pixel_to_square().
  (void)pixel;
  return chesspp::core::NO_SQUARE;
}

std::optional<chesspp::core::Move> InputHandler::find_move_to_square(
    const chesspp::core::MoveList& legal_moves,
    chesspp::core::Square to) const noexcept {
  // TODO: Scan legal_moves for selected_square_ -> to.
  (void)legal_moves;
  (void)to;
  return std::nullopt;
}

} // namespace chesspp::app
