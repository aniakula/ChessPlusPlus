#include "ui_panels.hpp"

#include "ui_layout.hpp"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>

namespace chesspp::app {

namespace {

constexpr float PANEL_PADDING = 14.0F;
constexpr float SECTION_GAP = 12.0F;
constexpr float EVAL_SECTION_HEIGHT = 108.0F;
constexpr float PROMOTION_TOP = 360.0F;
constexpr float PROMOTION_HEIGHT = 120.0F;

// ±4 pawns saturates the bar; soft curve keeps small edges readable.
constexpr float EVAL_BAR_SCALE_CP = 400.0F;

constexpr std::array<chesspp::core::PieceType, 4> PROMOTION_CHOICES{
    chesspp::core::PieceType::Queen, chesspp::core::PieceType::Rook,
    chesspp::core::PieceType::Bishop, chesspp::core::PieceType::Knight};

[[nodiscard]] const char *color_name(chesspp::core::Color color) {
  return color == chesspp::core::Color::White ? "White" : "Black";
}

[[nodiscard]] const char *
game_result_message(const chesspp::core::Game &game,
                    chesspp::core::Color human_color) {
  switch (game.result()) {
  case chesspp::core::GameResult::WhiteWins:
    return human_color == chesspp::core::Color::White ? "You win!"
                                                      : "You lose.";
  case chesspp::core::GameResult::BlackWins:
    return human_color == chesspp::core::Color::Black ? "You win!"
                                                      : "You lose.";
  case chesspp::core::GameResult::Draw:
    return "Draw.";
  case chesspp::core::GameResult::Ongoing:
    return "Game in progress.";
  }
  return "Unknown result.";
}

[[nodiscard]] float eval_white_fill_ratio(chesspp::core::Score evaluation) {
  const float normalized =
      static_cast<float>(evaluation) / EVAL_BAR_SCALE_CP;
  const float curved = std::tanh(normalized);
  return std::clamp(0.5F + 0.5F * curved, 0.02F, 0.98F);
}

[[nodiscard]] char promotion_letter(chesspp::core::PieceType piece) {
  switch (piece) {
  case chesspp::core::PieceType::Queen:
    return 'Q';
  case chesspp::core::PieceType::Rook:
    return 'R';
  case chesspp::core::PieceType::Bishop:
    return 'B';
  case chesspp::core::PieceType::Knight:
    return 'N';
  default:
    return '?';
  }
}

[[nodiscard]] sf::FloatRect
promotion_button_bounds(const sf::FloatRect &region, std::size_t index) {
  constexpr float button_gap = 8.0F;
  constexpr float button_height = 44.0F;
  const float content_width = region.size.x - 20.0F;
  const float button_width =
      (content_width - button_gap * 3.0F) / static_cast<float>(PROMOTION_CHOICES.size());
  const float x =
      region.position.x + 10.0F +
      static_cast<float>(index) * (button_width + button_gap);
  const float y = region.position.y + 52.0F;
  return {{x, y}, {button_width, button_height}};
}

} // namespace

sf::FloatRect UiPanels::promotion_region() const noexcept {
  return {{PANEL_PADDING, PROMOTION_TOP},
          {static_cast<float>(ui_layout::PANEL_WIDTH) - PANEL_PADDING * 2.0F,
           PROMOTION_HEIGHT}};
}

std::optional<chesspp::core::PieceType>
UiPanels::promotion_choice_at(sf::Vector2i pixel) const noexcept {
  const sf::Vector2f point{static_cast<float>(pixel.x),
                           static_cast<float>(pixel.y)};
  const sf::FloatRect region = promotion_region();
  for (std::size_t index = 0; index < PROMOTION_CHOICES.size(); ++index) {
    if (promotion_button_bounds(region, index).contains(point)) {
      return PROMOTION_CHOICES[index];
    }
  }
  return std::nullopt;
}

void UiPanels::draw_panel_background(sf::RenderWindow &window,
                                     const sf::FloatRect &region,
                                     const sf::Color &fill) const {
  sf::RectangleShape background{region.size};
  background.setPosition(region.position);
  background.setFillColor(fill);
  window.draw(background);
}

void UiPanels::draw_section_box(sf::RenderWindow &window, const sf::Font &font,
                                const sf::FloatRect &bounds, const char *title,
                                const char *body, const unsigned title_size,
                                const unsigned body_size) const {
  sf::RectangleShape frame{bounds.size};
  frame.setPosition(bounds.position);
  frame.setFillColor(sf::Color(28, 31, 38));
  frame.setOutlineColor(sf::Color(70, 78, 92));
  frame.setOutlineThickness(1.0F);
  window.draw(frame);

  sf::Text title_text{font, title, title_size};
  title_text.setFillColor(sf::Color(210, 214, 222));
  title_text.setPosition({bounds.position.x + 10.0F, bounds.position.y + 8.0F});
  window.draw(title_text);

  sf::Text body_text{font, body, body_size};
  body_text.setFillColor(sf::Color(150, 158, 170));
  body_text.setPosition({bounds.position.x + 10.0F, bounds.position.y + 32.0F});
  window.draw(body_text);
}

void UiPanels::draw_eval_bar(sf::RenderWindow &window, const sf::Font &font,
                             const sf::FloatRect &bounds,
                             chesspp::core::Score evaluation) const {
  sf::RectangleShape frame{bounds.size};
  frame.setPosition(bounds.position);
  frame.setFillColor(sf::Color(28, 31, 38));
  frame.setOutlineColor(sf::Color(70, 78, 92));
  frame.setOutlineThickness(1.0F);
  window.draw(frame);

  sf::Text title_text{font, "Evaluation", 16U};
  title_text.setFillColor(sf::Color(210, 214, 222));
  title_text.setPosition({bounds.position.x + 10.0F, bounds.position.y + 8.0F});
  window.draw(title_text);

  const float pawns = static_cast<float>(evaluation) / 100.0F;
  char score_buffer[32];
  if (evaluation > 0) {
    snprintf(score_buffer, sizeof(score_buffer), "+%.2f",
             static_cast<double>(pawns));
  } else if (evaluation < 0) {
    snprintf(score_buffer, sizeof(score_buffer), "%.2f",
             static_cast<double>(pawns));
  } else {
    snprintf(score_buffer, sizeof(score_buffer), "0.00");
  }

  sf::Text score_text{font, score_buffer, 22U};
  score_text.setFillColor(sf::Color(235, 238, 245));
  score_text.setPosition({bounds.position.x + 10.0F, bounds.position.y + 30.0F});
  window.draw(score_text);

  const char *advantage =
      evaluation > 15 ? "White advantage"
      : evaluation < -15
          ? "Black advantage"
          : "Equal position";
  sf::Text advantage_text{font, advantage, 12U};
  advantage_text.setFillColor(sf::Color(150, 158, 170));
  advantage_text.setPosition(
      {bounds.position.x + 10.0F, bounds.position.y + 58.0F});
  window.draw(advantage_text);

  constexpr float bar_height = 18.0F;
  constexpr float bar_margin_x = 10.0F;
  const float bar_width = bounds.size.x - bar_margin_x * 2.0F;
  const float bar_x = bounds.position.x + bar_margin_x;
  const float bar_y = bounds.position.y + bounds.size.y - bar_height - 12.0F;

  sf::RectangleShape black_track{{bar_width, bar_height}};
  black_track.setPosition({bar_x, bar_y});
  black_track.setFillColor(sf::Color(42, 36, 32));
  black_track.setOutlineColor(sf::Color(70, 78, 92));
  black_track.setOutlineThickness(1.0F);
  window.draw(black_track);

  const float white_ratio = eval_white_fill_ratio(evaluation);
  const float white_width = bar_width * white_ratio;
  sf::RectangleShape white_fill{{white_width, bar_height}};
  white_fill.setPosition({bar_x, bar_y});
  white_fill.setFillColor(sf::Color(240, 217, 181));
  window.draw(white_fill);

  sf::RectangleShape center_tick{{2.0F, bar_height}};
  center_tick.setPosition({bar_x + bar_width * 0.5F - 1.0F, bar_y});
  center_tick.setFillColor(sf::Color(120, 128, 140, 180));
  window.draw(center_tick);
}

void UiPanels::draw_promotion_chooser(sf::RenderWindow &window,
                                      const sf::Font &font,
                                      const sf::FloatRect &bounds) const {
  sf::RectangleShape frame{bounds.size};
  frame.setPosition(bounds.position);
  frame.setFillColor(sf::Color(28, 31, 38));
  frame.setOutlineColor(sf::Color(120, 160, 210));
  frame.setOutlineThickness(1.0F);
  window.draw(frame);

  sf::Text title_text{font, "Promote to", 16U};
  title_text.setFillColor(sf::Color(210, 214, 222));
  title_text.setPosition({bounds.position.x + 10.0F, bounds.position.y + 8.0F});
  window.draw(title_text);

  sf::Text hint_text{font, "Choose a piece", 12U};
  hint_text.setFillColor(sf::Color(150, 158, 170));
  hint_text.setPosition({bounds.position.x + 10.0F, bounds.position.y + 30.0F});
  window.draw(hint_text);

  for (std::size_t index = 0; index < PROMOTION_CHOICES.size(); ++index) {
    const sf::FloatRect button = promotion_button_bounds(bounds, index);
    sf::RectangleShape shape{button.size};
    shape.setPosition(button.position);
    shape.setFillColor(sf::Color(52, 58, 70));
    shape.setOutlineColor(sf::Color(88, 96, 112));
    shape.setOutlineThickness(1.0F);
    window.draw(shape);

    const char label[2] = {promotion_letter(PROMOTION_CHOICES[index]), '\0'};
    sf::Text label_text{font, label, 20U};
    label_text.setFillColor(sf::Color(235, 238, 245));
    const sf::FloatRect text_bounds = label_text.getLocalBounds();
    label_text.setPosition(
        {button.position.x + (button.size.x - text_bounds.size.x) / 2.0F -
             text_bounds.position.x,
         button.position.y + (button.size.y - text_bounds.size.y) / 2.0F -
             text_bounds.position.y});
    window.draw(label_text);
  }
}

void UiPanels::draw_left(sf::RenderWindow &window, const sf::Font &font,
                         const chesspp::core::Game &game,
                         const chesspp::core::Color human_color,
                         chesspp::core::Score evaluation,
                         bool awaiting_promotion) const {
  draw_panel_background(window, ui_layout::left_panel_region(),
                        sf::Color(48, 52, 60));

  sf::Text heading{font, "Chess++", 22U};
  heading.setFillColor(sf::Color(235, 238, 245));
  heading.setPosition({PANEL_PADDING, PANEL_PADDING});
  window.draw(heading);

  const chesspp::core::Color side = game.board().side_to_move();
  char turn_buffer[64];
  if (game.result() == chesspp::core::GameResult::Ongoing) {
    snprintf(turn_buffer, sizeof(turn_buffer), "%s to move", color_name(side));
  } else {
    snprintf(turn_buffer, sizeof(turn_buffer), "Game over");
  }

  float y = 56.0F;
  const float width =
      static_cast<float>(ui_layout::PANEL_WIDTH) - PANEL_PADDING * 2.0F;

  draw_section_box(window, font, {{PANEL_PADDING, y}, {width, 72.0F}}, "Turn",
                   turn_buffer);
  y += 72.0F + SECTION_GAP;

  char status_buffer[96];
  if (awaiting_promotion) {
    snprintf(status_buffer, sizeof(status_buffer), "Choose promotion.");
  } else if (game.is_checkmate()) {
    snprintf(status_buffer, sizeof(status_buffer), "Checkmate.");
  } else if (game.is_stalemate()) {
    snprintf(status_buffer, sizeof(status_buffer), "Stalemate.");
  } else if (game.is_check()) {
    snprintf(status_buffer, sizeof(status_buffer), "Check.");
  } else if (game.is_draw()) {
    snprintf(status_buffer, sizeof(status_buffer), "Draw: %s",
             game_result_message(game, human_color));
  } else {
    snprintf(status_buffer, sizeof(status_buffer), "%s",
             game_result_message(game, human_color));
  }

  draw_section_box(window, font, {{PANEL_PADDING, y}, {width, 72.0F}}, "Status",
                   status_buffer);
  y += 72.0F + SECTION_GAP;

  draw_eval_bar(window, font, {{PANEL_PADDING, y}, {width, EVAL_SECTION_HEIGHT}},
                evaluation);

  if (awaiting_promotion) {
    draw_promotion_chooser(window, font, promotion_region());
  }
}

void UiPanels::draw_right(sf::RenderWindow &window,
                          const sf::Font &font) const {
  (void)font;
  draw_panel_background(window, ui_layout::right_panel_region(),
                        sf::Color(32, 36, 42));
}

} // namespace chesspp::app
