#include "game.hpp"

namespace chesspp::core {

Game::Game() : board_{Board::starting_position()} { record_repetition_key(); }

Game::Game(Board board) : board_{board} { record_repetition_key(); }

const Board &Game::board() const noexcept { return board_; }

Board &Game::board() noexcept { return board_; }

MoveList Game::legal_moves() const {
  // TODO: MoveGenerator::generate_legal needs a mutable Board for make/unmake.
  Board copy = board_;
  MoveList moves;
  MoveGenerator::generate_legal(copy, moves);
  return moves;
}

bool Game::is_legal(Move move) const {
  // TODO: Check whether move appears in legal_moves().
  (void)move;
  return false;
}

bool Game::try_make_move(Move move) {
  // TODO: Validate, make move on board_, then record repetition key.
  (void)move;
  return false;
}

bool Game::is_check() const { return board_.in_check(board_.side_to_move()); }

bool Game::is_checkmate() const {
  // TODO: In check and no legal moves.
  return false;
}

bool Game::is_stalemate() const {
  // TODO: Not in check and no legal moves.
  return false;
}

bool Game::is_draw() const { return draw_reason() != DrawReason::None; }

DrawReason Game::draw_reason() const {
  // TODO: Check stalemate, fifty-move, repetition, insufficient material.
  return DrawReason::None;
}

GameResult Game::result() const {
  // TODO: Return win/loss/draw based on checkmate/draw state.
  return GameResult::Ongoing;
}

void Game::reset() {
  board_ = Board::starting_position();
  repetition_history_.clear();
  record_repetition_key();
}

void Game::load_position(Board board) {
  board_ = board;
  repetition_history_.clear();
  record_repetition_key();
}

void Game::record_repetition_key() {
  repetition_history_.push_back(board_.zobrist_key());
}

std::size_t Game::repetition_count(HashKey key) const {
  // TODO: Count matching keys in repetition_history_.
  (void)key;
  return 0;
}

bool Game::has_insufficient_material() const {
  // TODO: Detect king-only, king+bishop/knight, same-color bishops, etc.
  return false;
}

} // namespace chesspp::core
