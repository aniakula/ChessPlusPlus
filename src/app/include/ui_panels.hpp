#pragma once

#include "game.hpp"
#include "types.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

#include <optional>

namespace chesspp::app {

// Side chrome around the centered board: game status/eval on the left,
// quiet spacer panel on the right. Promotion choice lives on the left panel.
class UiPanels {
public:
  void draw_left(sf::RenderWindow &window, const sf::Font &font,
                 const chesspp::core::Game &game,
                 chesspp::core::Color human_color,
                 chesspp::core::Score evaluation,
                 bool awaiting_promotion) const;
  void draw_right(sf::RenderWindow &window, const sf::Font &font) const;

  [[nodiscard]] sf::FloatRect promotion_region() const noexcept;
  [[nodiscard]] std::optional<chesspp::core::PieceType>
  promotion_choice_at(sf::Vector2i pixel) const noexcept;

private:
  void draw_panel_background(sf::RenderWindow &window,
                             const sf::FloatRect &region,
                             const sf::Color &fill) const;
  void draw_section_box(sf::RenderWindow &window, const sf::Font &font,
                        const sf::FloatRect &bounds, const char *title,
                        const char *body, unsigned title_size = 16U,
                        unsigned body_size = 13U) const;
  void draw_eval_bar(sf::RenderWindow &window, const sf::Font &font,
                     const sf::FloatRect &bounds,
                     chesspp::core::Score evaluation) const;
  void draw_promotion_chooser(sf::RenderWindow &window, const sf::Font &font,
                              const sf::FloatRect &bounds) const;
};

} // namespace chesspp::app
