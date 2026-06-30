#pragma once

#include "game.hpp"
#include "types.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace chesspp::app {

// Side chrome around the centered board: metadata/options on the left,
// engine log terminal on the right. Popup anchors live on the left panel.
class UiPanels {
public:
  void draw_left(sf::RenderWindow &window, const sf::Font &font,
                 const chesspp::core::Game &game,
                 chesspp::core::Color human_color) const;
  void draw_right(sf::RenderWindow &window, const sf::Font &font) const;

  [[nodiscard]] sf::FloatRect popup_region() const noexcept;

private:
  void draw_panel_background(sf::RenderWindow &window,
                             const sf::FloatRect &region,
                             const sf::Color &fill) const;
  void draw_section_box(sf::RenderWindow &window, const sf::Font &font,
                        const sf::FloatRect &bounds, const char *title,
                        const char *body, unsigned title_size = 16U,
                        unsigned body_size = 13U) const;
  void draw_stub_button(sf::RenderWindow &window, const sf::Font &font,
                        const sf::FloatRect &bounds,
                        const char *label) const;
};

} // namespace chesspp::app
