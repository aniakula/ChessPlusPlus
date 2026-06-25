add_library(chesspp_core STATIC
    src/board.cpp
    src/movegen.cpp
    src/game.cpp
    src/pgn.cpp
)

target_include_directories(chesspp_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

chesspp_set_warnings(chesspp_core)
