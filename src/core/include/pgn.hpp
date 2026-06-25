#pragma once

#include "board.hpp"
#include "types.hpp"

#include <string>
#include <vector>

namespace chesspp::core {

[[nodiscard]] std::vector<Move> parse_pgn_moves(const std::string &pgn);
[[nodiscard]] std::vector<Move> parse_pgn_moves(const std::string &pgn,
                                                Board initial_position);

} // namespace chesspp::core
