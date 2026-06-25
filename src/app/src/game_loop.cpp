#include "game_loop.hpp"

namespace chesspp::app {

GameLoop::GameLoop()
    : window_{sf::VideoMode(800, 800), "Chess++"}, renderer_{window_} {}

void GameLoop::run() {
  // TODO: Main SFML loop:
  // 1. poll events
  // 2. pass events to InputHandler
  // 3. apply human move through core::Game
  // 4. ask Engine for a move when it is engine's turn
  // 5. render current state
}

void GameLoop::handle_event(const sf::Event& event) {
  // TODO: Close window or convert event to InputAction.
  (void)event;
}

void GameLoop::handle_human_action(const InputAction& action) {
  // TODO: Validate/apply moves through game_.try_make_move().
  (void)action;
}

void GameLoop::play_engine_turn() {
  // TODO: Call engine_.set_position(game_.board()), engine_.think(), then apply.
}

bool GameLoop::is_human_turn() const noexcept {
  return game_.board().side_to_move() == human_color_;
}

bool GameLoop::game_over() const {
  return game_.result() != chesspp::core::GameResult::Ongoing;
}

void startGame() {
  GameLoop loop;
  loop.run();
}

} // namespace chesspp::app
