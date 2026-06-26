#pragma once

#include "board.hpp"
#include "game.hpp"

#include "SFML/Graphics/RenderWindow.hpp"
#include <SFML/Graphics.hpp>

#include <optional>

namespace chesspp::app {

class Renderer {
public:
  explicit Renderer(sf::RenderWindow &window);

  bool load_assets(const char *asset_root);

  // Renderer should never own or mutate chess state. It reads Board/Game and
  // draws the current state, selected square, move hints, and game result.
  void draw(const chesspp::core::Game &game);
  void draw(const chesspp::core::Game &game,
            std::optional<chesspp::core::Square> selected_square,
            const chesspp::core::MoveList &legal_moves);
  void draw_board(const chesspp::core::Board &board);
  void draw_piece(chesspp::core::Square square, chesspp::core::Color color,
                  chesspp::core::PieceType piece);
  void draw_move_highlights(const chesspp::core::MoveList &legal_moves);
  void draw_status(const chesspp::core::Game &game);

  [[nodiscard]] chesspp::core::Square pixel_to_square(sf::Vector2i pixel) const;
  [[nodiscard]] sf::Vector2f
  square_to_pixel(chesspp::core::Square square) const;

private:
  sf::RenderWindow &window_;
  std::optional<sf::Font> font_{};
  std::optional<sf::Texture> piece_texture_{};

  void draw_square_highlight(chesspp::core::Square square, sf::Color color);
  [[nodiscard]] sf::IntRect piece_texture_rect(
      chesspp::core::Color color, chesspp::core::PieceType piece) const;
};

} // namespace chesspp::app
