#pragma once

#include "types.hpp"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace chesspp::app::ui_layout {

constexpr unsigned PANEL_WIDTH = 280U;
constexpr unsigned BOARD_SIZE = 800U;
constexpr unsigned TILE_SIZE = BOARD_SIZE / 8U;
constexpr unsigned WINDOW_WIDTH = PANEL_WIDTH + BOARD_SIZE + PANEL_WIDTH;
constexpr unsigned WINDOW_HEIGHT = BOARD_SIZE;

constexpr float BOARD_ORIGIN_X = static_cast<float>(PANEL_WIDTH);
constexpr float BOARD_ORIGIN_Y = 0.0F;

[[nodiscard]] inline bool contains_board_pixel(sf::Vector2i pixel) noexcept {
  const float px = static_cast<float>(pixel.x);
  const float py = static_cast<float>(pixel.y);
  return px >= BOARD_ORIGIN_X && px < BOARD_ORIGIN_X + BOARD_SIZE && py >= 0.0F &&
         py < static_cast<float>(BOARD_SIZE);
}

[[nodiscard]] inline chesspp::core::Square
pixel_to_square(sf::Vector2i pixel) noexcept {
  if (!contains_board_pixel(pixel)) {
    return chesspp::core::NO_SQUARE;
  }

  const int local_x = pixel.x - static_cast<int>(BOARD_ORIGIN_X);
  const int local_y = pixel.y - static_cast<int>(BOARD_ORIGIN_Y);
  const int file = local_x / static_cast<int>(TILE_SIZE);
  const int screen_rank = local_y / static_cast<int>(TILE_SIZE);
  const int rank = 7 - screen_rank;

  if (file < 0 || file >= 8 || rank < 0 || rank >= 8) {
    return chesspp::core::NO_SQUARE;
  }

  return chesspp::core::square_from(file, rank);
}

[[nodiscard]] inline sf::Vector2f
square_to_pixel(chesspp::core::Square square) noexcept {
  if (square == chesspp::core::NO_SQUARE) {
    return {};
  }

  const int file = static_cast<int>(square % 8);
  const int rank = static_cast<int>(square / 8);
  return {BOARD_ORIGIN_X + static_cast<float>(file) * static_cast<float>(TILE_SIZE),
          BOARD_ORIGIN_Y +
              static_cast<float>(7 - rank) * static_cast<float>(TILE_SIZE)};
}

[[nodiscard]] inline sf::FloatRect left_panel_region() noexcept {
  return {{0.0F, 0.0F},
          {static_cast<float>(PANEL_WIDTH), static_cast<float>(WINDOW_HEIGHT)}};
}

[[nodiscard]] inline sf::FloatRect right_panel_region() noexcept {
  return {{BOARD_ORIGIN_X + static_cast<float>(BOARD_SIZE), 0.0F},
          {static_cast<float>(PANEL_WIDTH), static_cast<float>(WINDOW_HEIGHT)}};
}

} // namespace chesspp::app::ui_layout
