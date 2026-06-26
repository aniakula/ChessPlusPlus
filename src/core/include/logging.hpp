#pragma once
#include <iostream>

namespace chesspp::core {

template <typename T> void log_debug(T &&message) {
  std::clog << message << '\n';
}

template <typename T> void log_debug(std::string &&label, T &&message) {
  std::clog << label << message << '\n';
}

} // namespace chesspp::core

#ifdef NDEBUG

#define DEBUG_LOG(message)
#define LABELED_DEBUG_LOG(label, message)

#else

#define DEBUG_LOG(message) ::chesspp::core::log_debug((message))
#define LABELED_DEBUG_LOG(label, message)                                      \
  ::chesspp::core::log_debug((label), (message))

#endif
