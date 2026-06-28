#include "game.hpp"
#include "movegen.hpp"
#include "types.hpp"

namespace chesspp::core {

Game::Game() : board_{Board::starting_position()} { record_repetition_key(); }

Game::Game(Board board) : board_{board} { record_repetition_key(); }

const Board &Game::board() const noexcept { return board_; }

Board &Game::board() noexcept { return board_; }

MoveList Game::legal_moves() const {
  Board copy = board_;
  MoveList moves;
  MoveGenerator::generate_legal(copy, moves);
  return moves;
}

bool Game::is_legal(Move move) const {
  const MoveList moves = legal_moves();
  return moves.contains(move);
}

bool Game::try_make_move(Move move) {
  if (!is_legal(move)) {
    return false;
  }

  UndoState undo;
  board_.make_move(move, undo);
  record_repetition_key();
  return true;
}

bool Game::is_check() const { return board_.in_check(board_.side_to_move()); }

bool Game::is_checkmate() const { return is_check() && legal_moves().empty(); }

bool Game::is_stalemate() const { return !is_check() && legal_moves().empty(); }

bool Game::is_draw() const { return draw_reason() != DrawReason::None; }

DrawReason Game::draw_reason() const {
  if (repetition_count(board_.zobrist_key()) >= 3) {
    return DrawReason::ThreefoldRepetition;
  } else if (is_stalemate()) {
    return DrawReason::Stalemate;
  } else if (board_.halfmove_clock() == 50) {
    return DrawReason::FiftyMoveRule;
  } else if (has_insufficient_material()) {
    return DrawReason::InsufficientMaterial;
  }

  return DrawReason::None;
}

GameResult Game::result() const {
  if (is_draw()) {
    return GameResult::Draw;
  } else if (is_checkmate()) {
    // The side to move has been mated, so the other side wins.
    return board_.side_to_move() == Color::White ? GameResult::BlackWins
                                                 : GameResult::WhiteWins;
  }

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
  const std::size_t reversible_position_count =
      static_cast<std::size_t>(board_.halfmove_clock()) + 1;
  // start from last reversible move to check for repitition
  const std::size_t first_index =
      repetition_history_.size() > reversible_position_count
          ? repetition_history_.size() - reversible_position_count
          : 0;

  std::size_t count = 0;
  for (std::size_t index = first_index; index < repetition_history_.size();
       ++index) {
    if (repetition_history_[index] == key) {
      ++count;
    }
  }

  return count;
}

bool Game::has_insufficient_material() const {
  // TODO: Detect king-only, king+bishop/knight, same-color bishops, etc.
  const Color side = board_.side_to_move();
  const Color opponent = opposite(side);

  const Bitboard side_pawns = board_.pieces(side, PieceType::Pawn);
  const Bitboard opponent_pawns = board_.pieces(opponent, PieceType::Pawn);
  const Bitboard side_queens = board_.pieces(side, PieceType::Queen);
  const Bitboard opponent_queens = board_.pieces(opponent, PieceType::Queen);
  const Bitboard side_rooks = board_.pieces(side, PieceType::Rook);
  const Bitboard opponent_rooks = board_.pieces(opponent, PieceType::Rook);
  const Bitboard side_bishops = board_.pieces(side, PieceType::Bishop);
  const Bitboard opponent_bishops = board_.pieces(opponent, PieceType::Bishop);
  const Bitboard side_knights = board_.pieces(side, PieceType::Knight);
  const Bitboard opponent_knights = board_.pieces(opponent, PieceType::Knight);

  const auto has_at_least_two = [](Bitboard n) { return (n & (n - 1)) != 0; };

  if (side_pawns || opponent_pawns || side_queens || opponent_queens ||
      side_rooks || opponent_rooks || has_at_least_two(side_bishops) ||
      has_at_least_two(opponent_bishops) || (side_bishops && side_knights) ||
      (opponent_bishops && opponent_knights)) {
    return false;
  }

  return true;
}

} // namespace chesspp::core
