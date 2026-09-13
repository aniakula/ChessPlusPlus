#include "game_loop.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Window/Event.hpp"
#include "game.hpp"
#include "logging.hpp"
#include "search.hpp"
#include "types.hpp"
#include "ui_layout.hpp"
#include <filesystem>
#include <optional>
#include <stdexcept>

namespace chesspp::app {

GameLoop::GameLoop()
    : window_{sf::VideoMode(
                  {ui_layout::WINDOW_WIDTH, ui_layout::WINDOW_HEIGHT}),
              "Chess++", sf::Style::Titlebar | sf::Style::Close},
      renderer_{window_} {
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

      if (const auto *mouse_press =
              event->getIf<sf::Event::MouseButtonPressed>()) {
        if (mouse_press->button == sf::Mouse::Button::Left &&
            pending_promotions_.has_value() &&
            handle_promotion_click(mouse_press->position)) {
          continue;
        }
      }

      // Ignore board input while a promotion choice is outstanding.
      if (pending_promotions_.has_value()) {
        continue;
      }

      const InputAction action =
          input_.transform_event(*event, game_, human_color_);
      handle_action(action);
    }

    draw_frame();

    if (!game_over() && !is_human_turn() && !pending_promotions_.has_value()) {
      play_engine_turn();
    }

    draw_frame();
  }
}

void GameLoop::handle_action(const InputAction &action) {
  switch (action.type) {
  case InputAction::Type::None:
    break;
  case InputAction::Type::SelectSquare:
    break;
  case InputAction::Type::RequestPromotion:
    pending_promotions_ = action.legal_moves;
    break;
  case InputAction::Type::Quit:
    window_.close();
    break;
  case InputAction::Type::PlayMove:
    if (game_.try_make_move(action.move)) {
      clear_pending_promotion();
      input_.clear_selection();
    }
    break;
  }
}

bool GameLoop::handle_promotion_click(sf::Vector2i pixel) {
  if (!pending_promotions_.has_value()) {
    return false;
  }

  const auto choice = renderer_.promotion_choice_at(pixel);
  if (!choice.has_value()) {
    // Click outside the chooser cancels promotion and clears selection.
    clear_pending_promotion();
    input_.clear_selection();
    return true;
  }

  for (const chesspp::core::Move move : *pending_promotions_) {
    if (move.promotion() == *choice) {
      if (game_.try_make_move(move)) {
        clear_pending_promotion();
        input_.clear_selection();
      }
      return true;
    }
  }

  return true;
}

void GameLoop::clear_pending_promotion() noexcept {
  pending_promotions_.reset();
}

void GameLoop::draw_frame() {
  renderer_.draw(game_, input_.selected_square(), input_.legal_moves(),
                 human_color_, pending_promotions_.has_value());
}

void GameLoop::play_engine_turn() {
  engine_.set_position(game_.board());
  engine::SearchLimits limits;
  const chesspp::engine::SearchResult &result = engine_.think(limits);

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
