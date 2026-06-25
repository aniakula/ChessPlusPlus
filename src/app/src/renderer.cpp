#include "renderer.hpp"

namespace chesspp::app {

Renderer::Renderer(sf::RenderWindow& window) : window_{window} {}

bool Renderer::load_assets(const char* asset_root) {
  // TODO: Load piece textures/fonts. Keep asset ownership in Renderer.
  (void)asset_root;
  return true;
}

void Renderer::draw(const chesspp::core::Game& game) {
  // TODO: Clear window, draw board/pieces/highlights/status, display.
  (void)window_;
  (void)game;
}

void Renderer::draw_board(const chesspp::core::Board& board) {
  // TODO: Draw 64 alternating SFML rectangles.
  (void)board;
}

void Renderer::draw_piece(chesspp::core::Square square,
                          chesspp::core::Color color,
                          chesspp::core::PieceType piece) {
  // TODO: Draw sprite matching piece/color at square.
  (void)square;
  (void)color;
  (void)piece;
}

void Renderer::draw_move_highlights(
    const chesspp::core::MoveList& legal_moves) {
  // TODO: Draw legal destination markers for selected piece.
  (void)legal_moves;
}

void Renderer::draw_status(const chesspp::core::Game& game) {
  // TODO: Draw side-to-move, check, result, engine-thinking text.
  (void)game;
}

chesspp::core::Square Renderer::pixel_to_square(sf::Vector2i pixel) const {
  // TODO: Convert window coordinates to 0..63 square index.
  (void)pixel;
  return chesspp::core::NO_SQUARE;
}

sf::Vector2f Renderer::square_to_pixel(chesspp::core::Square square) const {
  // TODO: Convert 0..63 square index to upper-left pixel coordinate.
  (void)square;
  return {};
}

} // namespace chesspp::app
