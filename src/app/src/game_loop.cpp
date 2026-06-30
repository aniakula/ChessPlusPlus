#include "game_loop.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Window/Event.hpp"
#include "logging.hpp"
#include "search.hpp"
#include <filesystem>
#include <optional>
#include <stdexcept>

namespace chesspp::app {

GameLoop::GameLoop()
    : window_{sf::VideoMode({800U, 800U}), "Chess++"}, renderer_{window_} {
  sf::Image icon;
  if (icon.loadFromFile(std::filesystem::path{"src/assets/ChessPPIcon.png"})) {
    window_.setIcon(icon.getSize(), icon.getPixelsPtr());
    window_.setFramerateLimit(60);
  }

  (void)renderer_.load_assets("src/assets");
}

void GameLoop::run() {
  while (window_.isOpen()) {
    while (const std::optional<sf::Event> event = window_.pollEvent()) {
      if ((*event).is<sf::Event::Closed>()) {
        window_.close();
        break;
      }
      const InputAction action =
          input_.transform_event(*event, game_, human_color_);
      handle_action(action);
    }

    if (!game_over() && !is_human_turn()) {
      play_engine_turn();
    }

    renderer_.draw(game_, input_.selected_square(), input_.legal_moves(),
                   human_color_);
  }
}

void GameLoop::handle_action(const InputAction &action) {
  switch (action.type) {
  case InputAction::Type::None:
    break;
  case InputAction::Type::SelectSquare:
  case InputAction::Type::RequestPromotion:
    break;
  case InputAction::Type::Quit:
    window_.close();
    break;
  case InputAction::Type::PlayMove:
    if (game_.try_make_move(action.move)) {
      input_.clear_selection();
    }
    break;
  }
}

void GameLoop::play_engine_turn() {
  engine_.set_position(game_.board());
  const chesspp::engine::SearchResult &result = engine_.think();

  LABELED_DEBUG_LOG("Best Move: ", result.best_move);
  LABELED_DEBUG_LOG("Score: ", result.score);
  LABELED_DEBUG_LOG("Max Depth Reached: ", result.depth_reached);
  LABELED_DEBUG_LOG("Search Stats: ", result.stats);

  if (!result.best_move.is_null()) {
    const bool move_applied = game_.try_make_move(result.best_move);
    if (!move_applied) {
      throw std::runtime_error("Error attempting to make engine move");
    }
  } else {
    throw std::runtime_error("Error null best move from engine");
  }
}

bool GameLoop::is_human_turn() const noexcept {
  return game_.board().side_to_move() == human_color_;
}

bool GameLoop::game_over() const {
  return game_.result() != chesspp::core::GameResult::Ongoing;
}

} // namespace chesspp::app
