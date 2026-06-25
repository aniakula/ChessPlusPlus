#include "engine.hpp"

namespace chesspp::engine {

Engine::Engine() = default;

void Engine::set_position(const chesspp::core::Board& board) { board_ = board; }

void Engine::set_search_limits(SearchLimits limits) noexcept { limits_ = limits; }

void Engine::set_transposition_table_size(std::size_t megabytes) {
  search_.resize_cache(megabytes);
}

void Engine::clear_cache() noexcept { search_.clear_cache(); }

SearchResult Engine::think() { return search_.search(board_, limits_); }

SearchResult Engine::think(const SearchLimits& limits) {
  return search_.search(board_, limits);
}

} // namespace chesspp::engine
