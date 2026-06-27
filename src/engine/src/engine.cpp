#include "engine.hpp"
#include <cstdint>

namespace chesspp::engine {

Engine::Engine() = default;

void Engine::set_position(const chesspp::core::Board &board) { board_ = board; }

void Engine::set_search_limits(SearchLimits limits) noexcept {
  limits_ = limits;
}

void Engine::set_search_timeout(uint64_t time_ms) {
  limits_.max_time_ms = time_ms;
}

void Engine::set_max_depth(int max_depth) { limits_.max_depth = max_depth; }

void Engine::set_max_nodes(uint64_t max_nodes) {
  limits_.max_nodes = max_nodes;
}

void Engine::set_transposition_table_size(std::size_t megabytes) {
  search_.resize_cache(megabytes);
}

void Engine::clear_cache() noexcept { search_.clear_cache(); }

SearchResult Engine::think() { return search_.search(board_, limits_); }

SearchResult Engine::think(const SearchLimits &limits) {
  return search_.search(board_, limits);
}

} // namespace chesspp::engine
