#include "ui_panels.hpp"

#include "ui_layout.hpp"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace chesspp::app {

namespace {

constexpr float PANEL_PADDING = 14.0F;
constexpr float SECTION_GAP = 12.0F;
constexpr float EVAL_SECTION_HEIGHT = 108.0F;
constexpr float DIALOGS_TOP = 436.0F;
constexpr float DIALOGS_HEIGHT = 140.0F;

// ±4 pawns saturates the bar; soft curve keeps small edges readable.
constexpr float EVAL_BAR_SCALE_CP = 400.0F;

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

} // namespace

sf::FloatRect UiPanels::popup_region() const noexcept {
  return {{PANEL_PADDING, DIALOGS_TOP},
          {static_cast<float>(ui_layout::PANEL_WIDTH) - PANEL_PADDING * 2.0F,
           DIALOGS_HEIGHT}};
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

void UiPanels::draw_stub_button(sf::RenderWindow &window, const sf::Font &font,
                                const sf::FloatRect &bounds,
                                const char *label) const {
  sf::RectangleShape button{bounds.size};
  button.setPosition(bounds.position);
  button.setFillColor(sf::Color(52, 58, 70));
  button.setOutlineColor(sf::Color(88, 96, 112));
  button.setOutlineThickness(1.0F);
  window.draw(button);

  sf::Text label_text{font, label, 13U};
  label_text.setFillColor(sf::Color(170, 176, 188));
  const sf::FloatRect text_bounds = label_text.getLocalBounds();
  label_text.setPosition(
      {bounds.position.x + (bounds.size.x - text_bounds.size.x) / 2.0F -
           text_bounds.position.x,
       bounds.position.y + (bounds.size.y - text_bounds.size.y) / 2.0F -
           text_bounds.position.y});
  window.draw(label_text);
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

  // Center tick at equal evaluation.
  sf::RectangleShape center_tick{{2.0F, bar_height}};
  center_tick.setPosition({bar_x + bar_width * 0.5F - 1.0F, bar_y});
  center_tick.setFillColor(sf::Color(120, 128, 140, 180));
  window.draw(center_tick);
}

void UiPanels::draw_left(sf::RenderWindow &window, const sf::Font &font,
                         const chesspp::core::Game &game,
                         const chesspp::core::Color human_color,
                         chesspp::core::Score evaluation) const {
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

  char player_buffer[64];
  snprintf(player_buffer, sizeof(player_buffer), "You play as %s",
           color_name(human_color));

  float y = 56.0F;
  const float width =
      static_cast<float>(ui_layout::PANEL_WIDTH) - PANEL_PADDING * 2.0F;

  draw_section_box(window, font, {{PANEL_PADDING, y}, {width, 72.0F}}, "Turn",
                   turn_buffer);
  y += 72.0F + SECTION_GAP;

  char status_buffer[96];
  if (game.is_checkmate()) {
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
  y += EVAL_SECTION_HEIGHT + SECTION_GAP;

  draw_section_box(window, font, {{PANEL_PADDING, y}, {width, 64.0F}}, "Player",
                   player_buffer);

  const sf::FloatRect popup = popup_region();
  draw_section_box(window, font, popup, "Dialogs", "");
  y = popup.position.y + popup.size.y + SECTION_GAP;

  draw_section_box(window, font, {{PANEL_PADDING, y}, {width, 72.0F}},
                   "Options", "Game controls (coming soon).");
  y += 72.0F + SECTION_GAP;

  const float button_height = 30.0F;
  draw_stub_button(window, font, {{PANEL_PADDING, y}, {width, button_height}},
                   "New Game");
  draw_stub_button(
      window, font,
      {{PANEL_PADDING, y + button_height + 8.0F}, {width, button_height}},
      "Resign");
}

void UiPanels::draw_right(sf::RenderWindow &window,
                          const sf::Font &font) const {
  draw_panel_background(window, ui_layout::right_panel_region(),
                        sf::Color(32, 36, 42));

  sf::Text heading{font, "Engine Log", 20U};
  heading.setFillColor(sf::Color(220, 224, 232));
  heading.setPosition({ui_layout::BOARD_ORIGIN_X +
                           static_cast<float>(ui_layout::BOARD_SIZE) +
                           PANEL_PADDING,
                       PANEL_PADDING});
  window.draw(heading);

  const float panel_x =
      ui_layout::BOARD_ORIGIN_X + static_cast<float>(ui_layout::BOARD_SIZE);
  const sf::FloatRect terminal_bounds{
      {panel_x + PANEL_PADDING, 52.0F},
      {static_cast<float>(ui_layout::PANEL_WIDTH) - PANEL_PADDING * 2.0F,
       static_cast<float>(ui_layout::WINDOW_HEIGHT) - 66.0F}};

  sf::RectangleShape terminal{terminal_bounds.size};
  terminal.setPosition(terminal_bounds.position);
  terminal.setFillColor(sf::Color(18, 20, 24));
  terminal.setOutlineColor(sf::Color(58, 64, 76));
  terminal.setOutlineThickness(1.0F);
  window.draw(terminal);

  const char *placeholder_lines[] = {
      "> engine ready",
      "> waiting for search output...",
      "> best move: (pending)",
      "> depth: --",
      "> nodes: --",
      "",
      "Events and diagnostics will stream here.",
  };

  float line_y = terminal_bounds.position.y + 10.0F;
  for (const char *line : placeholder_lines) {
    sf::Text line_text{font, line, 12U};
    line_text.setFillColor(sf::Color(118, 196, 141));
    line_text.setPosition({terminal_bounds.position.x + 10.0F, line_y});
    window.draw(line_text);
    line_y += 18.0F;
  }
}

} // namespace chesspp::app
