#pragma once

#include "engine.hpp"
#include "game.hpp"
#include "input.hpp"
#include "renderer.hpp"
#include "types.hpp"

#include <SFML/Graphics.hpp>

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

  void handle_action(const InputAction &action);
  void play_engine_turn();
  [[nodiscard]] bool is_human_turn() const noexcept;
  [[nodiscard]] bool game_over() const;
};

} // namespace chesspp::app
