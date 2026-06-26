#include "search.hpp"

#include <gtest/gtest.h>

#include <sstream>

namespace chesspp::engine {
namespace {

TEST(SearchTest, SearchStatsStreamOutputIsHumanReadable) {
  SearchStats stats{};
  stats.nodes = 123;
  stats.transposition_hits = 4;
  stats.beta_cutoffs = 5;

  std::ostringstream out;
  out << stats;

  EXPECT_EQ(out.str(),
            "SearchStats{nodes=123, transposition_hits=4, beta_cutoffs=5}");
}

} // namespace
} // namespace chesspp::engine
