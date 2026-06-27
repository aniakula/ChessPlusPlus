#pragma once

#include "board.hpp"
#include "search.hpp"
#include "types.hpp"

#include <cstddef>
#include <cstdint>

namespace chesspp::engine {

class Engine {
public:
  Engine();

  void set_position(const chesspp::core::Board &board);
  void set_search_limits(SearchLimits limits) noexcept;
  void set_search_timeout(uint64_t time_ms);
  void set_max_depth(int max_depth);
  void set_max_nodes(uint64_t max_nodes);
  void set_transposition_table_size(std::size_t megabytes);
  void clear_cache() noexcept;

  [[nodiscard]] SearchResult think();
  [[nodiscard]] SearchResult think(const SearchLimits &limits);

private:
  chesspp::core::Board board_{};
  SearchLimits limits_{};
  Search search_{};
};

} // namespace chesspp::engine
