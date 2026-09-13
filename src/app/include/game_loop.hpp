#pragma once

#include "engine.hpp"
#include "game.hpp"
#include "input.hpp"
#include "movegen.hpp"
#include "renderer.hpp"
#include "types.hpp"

#include <SFML/Graphics.hpp>

#include <optional>

namespace chesspp::app {

class GameLoop {
public:
  GameLoop();

  void run();

private:
  chesspp::core::Color human_color_{chesspp::core::Color::White};
  sf::RenderWindow window_;
  chesspp::core::Game game_{};
  chesspp::engine::Engine engine_{};
  Renderer renderer_;
  InputHandler input_{};
  std::optional<chesspp::core::MoveList> pending_promotions_{};

  void handle_action(const InputAction &action);
  bool handle_promotion_click(sf::Vector2i pixel);
  void clear_pending_promotion() noexcept;
  void play_engine_turn();
  void draw_frame();
  [[nodiscard]] bool is_human_turn() const noexcept;
  [[nodiscard]] bool game_over() const;
};

} // namespace chesspp::app
