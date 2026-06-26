#include "renderer.hpp"

#include <cmath>
#include <filesystem>

namespace {

constexpr float BOARD_SIZE = 800.0F;
constexpr float TILE_SIZE = BOARD_SIZE / 8.0F;

[[nodiscard]] char piece_letter(chesspp::core::Color color,
                                chesspp::core::PieceType piece) {
  char letter = '?';
  switch (piece) {
  case chesspp::core::PieceType::Pawn:
    letter = 'P';
    break;
  case chesspp::core::PieceType::Knight:
    letter = 'N';
    break;
  case chesspp::core::PieceType::Bishop:
    letter = 'B';
    break;
  case chesspp::core::PieceType::Rook:
    letter = 'R';
    break;
  case chesspp::core::PieceType::Queen:
    letter = 'Q';
    break;
  case chesspp::core::PieceType::King:
    letter = 'K';
    break;
  case chesspp::core::PieceType::None:
    letter = '?';
    break;
  }

  if (color == chesspp::core::Color::Black && letter >= 'A' && letter <= 'Z') {
    letter = static_cast<char>(letter - 'A' + 'a');
  }

  return letter;
}

} // namespace

namespace chesspp::app {

Renderer::Renderer(sf::RenderWindow &window)
    : window_{window}, font_{"src/assets/fonts/Roboto-SemiBold.ttf"} {}

bool Renderer::load_assets(const char *asset_root) {
  const std::filesystem::path root{asset_root};

  sf::Texture texture;
  if (texture.loadFromFile(root / "sprites" / "chess_set.png")) {
    texture.setSmooth(true);
    piece_texture_ = std::move(texture);
  }

  if (!font_.has_value()) {
    font_ = sf::Font{root / "fonts" / "Roboto-SemiBold.ttf"};
  }

  return piece_texture_.has_value() && font_.has_value();
}

void Renderer::draw(const chesspp::core::Game &game) {
  draw(game, std::nullopt, {});
}

void Renderer::draw(const chesspp::core::Game &game,
                    std::optional<chesspp::core::Square> selected_square,
                    const chesspp::core::MoveList &legal_moves) {
  window_.clear(sf::Color::Black);
  draw_board(game.board());

  if (selected_square.has_value()) {
    draw_square_highlight(*selected_square, sf::Color(255, 215, 0, 110));
  }

  draw_move_highlights(legal_moves);

  for (int square = 0; square < chesspp::core::SQUARE_COUNT; ++square) {
    const auto piece = game.board().piece_and_color_on(
        static_cast<chesspp::core::Square>(square));
    if (piece.has_value()) {
      draw_piece(static_cast<chesspp::core::Square>(square), piece->first,
                 piece->second);
    }
  }

  draw_status(game);
  window_.display();
}

void Renderer::draw_board(const chesspp::core::Board &board) {
  (void)board;

  const sf::Color light{240, 217, 181};
  const sf::Color dark{181, 136, 99};

  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      sf::RectangleShape square{{TILE_SIZE, TILE_SIZE}};
      square.setPosition({static_cast<float>(file) * TILE_SIZE,
                          static_cast<float>(7 - rank) * TILE_SIZE});
      square.setFillColor(((file + rank) % 2 == 0) ? light : dark);
      window_.draw(square);
    }
  }
}

void Renderer::draw_piece(chesspp::core::Square square,
                          chesspp::core::Color color,
                          chesspp::core::PieceType piece) {
  const sf::Vector2f position = square_to_pixel(square);

  if (piece_texture_.has_value()) {
    const sf::IntRect texture_rect = piece_texture_rect(color, piece);
    sf::Sprite sprite{*piece_texture_, texture_rect};

    const sf::FloatRect bounds = sprite.getLocalBounds();
    const float scale =
        std::min(TILE_SIZE / bounds.size.x, TILE_SIZE / bounds.size.y) * 0.9F;

    sprite.setScale({scale, scale});
    sprite.setOrigin({bounds.position.x + bounds.size.x / 2.0F,
                      bounds.position.y + bounds.size.y / 2.0F});
    sprite.setPosition(
        {position.x + TILE_SIZE / 2.0F, position.y + TILE_SIZE / 2.0F});
    window_.draw(sprite);
    return;
  }

  sf::CircleShape backing{TILE_SIZE * 0.34F};
  backing.setPosition(
      {position.x + (TILE_SIZE - backing.getRadius() * 2.0F) / 2.0F,
       position.y + (TILE_SIZE - backing.getRadius() * 2.0F) / 2.0F});
  backing.setFillColor(color == chesspp::core::Color::White
                           ? sf::Color(245, 245, 245, 220)
                           : sf::Color(25, 25, 25, 220));
  backing.setOutlineThickness(2.0F);
  backing.setOutlineColor(color == chesspp::core::Color::White
                              ? sf::Color(30, 30, 30)
                              : sf::Color(230, 230, 230));
  window_.draw(backing);

  if (!font_.has_value()) {
    return;
  }

  sf::Text text{*font_, sf::String(piece_letter(color, piece)), 52U};
  text.setFillColor(color == chesspp::core::Color::White ? sf::Color::Black
                                                         : sf::Color::White);

  const sf::FloatRect bounds = text.getLocalBounds();
  text.setOrigin({bounds.position.x + bounds.size.x / 2.0F,
                  bounds.position.y + bounds.size.y / 2.0F});
  text.setPosition(
      {position.x + TILE_SIZE / 2.0F, position.y + TILE_SIZE / 2.0F});
  window_.draw(text);
}

void Renderer::draw_move_highlights(
    const chesspp::core::MoveList &legal_moves) {
  for (const chesspp::core::Move move : legal_moves) {
    draw_square_highlight(move.to(), sf::Color(70, 130, 180, 120));
  }
}

void Renderer::draw_status(const chesspp::core::Game &game) { (void)game; }

chesspp::core::Square Renderer::pixel_to_square(sf::Vector2i pixel) const {
  if (pixel.x < 0 || pixel.y < 0 || static_cast<float>(pixel.x) >= BOARD_SIZE ||
      static_cast<float>(pixel.y) >= BOARD_SIZE) {
    return chesspp::core::NO_SQUARE;
  }

  const int file =
      static_cast<int>(std::floor(static_cast<float>(pixel.x) / TILE_SIZE));
  const int screen_rank =
      static_cast<int>(std::floor(static_cast<float>(pixel.y) / TILE_SIZE));
  const int rank = 7 - screen_rank;

  if (file < 0 || file >= 8 || rank < 0 || rank >= 8) {
    return chesspp::core::NO_SQUARE;
  }

  return chesspp::core::square_from(file, rank);
}

sf::Vector2f Renderer::square_to_pixel(chesspp::core::Square square) const {
  if (square == chesspp::core::NO_SQUARE) {
    return {};
  }

  const int file = static_cast<int>(square % 8);
  const int rank = static_cast<int>(square / 8);
  return {static_cast<float>(file) * TILE_SIZE,
          static_cast<float>(7 - rank) * TILE_SIZE};
}

void Renderer::draw_square_highlight(chesspp::core::Square square,
                                     sf::Color color) {
  if (square == chesspp::core::NO_SQUARE) {
    return;
  }

  sf::RectangleShape highlight{{TILE_SIZE, TILE_SIZE}};
  highlight.setPosition(square_to_pixel(square));
  highlight.setFillColor(color);
  window_.draw(highlight);
}

sf::IntRect Renderer::piece_texture_rect(chesspp::core::Color color,
                                         chesspp::core::PieceType piece) const {
  const sf::Vector2u texture_size = piece_texture_->getSize();
  const int cell_width = static_cast<int>(texture_size.x / 6U);
  const int cell_height = static_cast<int>(texture_size.y / 2U);

  int column = 0;
  switch (piece) {
  case chesspp::core::PieceType::King:
    column = 0;
    break;
  case chesspp::core::PieceType::Queen:
    column = 1;
    break;
  case chesspp::core::PieceType::Bishop:
    column = 2;
    break;
  case chesspp::core::PieceType::Knight:
    column = 3;
    break;
  case chesspp::core::PieceType::Rook:
    column = 4;
    break;
  case chesspp::core::PieceType::Pawn:
    column = 5;
    break;
  case chesspp::core::PieceType::None:
    column = 0;
    break;
  }

  const int row = color == chesspp::core::Color::White ? 0 : 1;
  return {{column * cell_width, row * cell_height}, {cell_width, cell_height}};
}

} // namespace chesspp::app
