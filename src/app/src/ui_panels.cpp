#include "ui_panels.hpp"

#include "ui_layout.hpp"

#include <SFML/Graphics.hpp>
#include <cstdio>

namespace chesspp::app {

namespace {

constexpr float PANEL_PADDING = 14.0F;
constexpr float SECTION_GAP = 12.0F;

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

} // namespace

sf::FloatRect UiPanels::popup_region() const noexcept {
  constexpr float top = 300.0F;
  constexpr float height = 180.0F;
  return {{PANEL_PADDING, top},
          {static_cast<float>(ui_layout::PANEL_WIDTH) - PANEL_PADDING * 2.0F,
           height}};
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

void UiPanels::draw_left(sf::RenderWindow &window, const sf::Font &font,
                         const chesspp::core::Game &game,
                         const chesspp::core::Color human_color) const {
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

  draw_section_box(window, font, {{PANEL_PADDING, y}, {width, 88.0F}}, "Status",
                   status_buffer);
  y += 88.0F + SECTION_GAP;

  draw_section_box(window, font, {{PANEL_PADDING, y}, {width, 72.0F}}, "Player",
                   player_buffer);
  y += 72.0F + SECTION_GAP;

  const sf::FloatRect popup = popup_region();
  draw_section_box(window, font, popup, "Dialogs", "");
  y = popup.position.y + popup.size.y + SECTION_GAP;

  draw_section_box(window, font, {{PANEL_PADDING, y}, {width, 92.0F}},
                   "Options", "Game controls (coming soon).");
  y += 92.0F + SECTION_GAP;

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
