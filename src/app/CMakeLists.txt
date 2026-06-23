add_executable(chesspp
    src/main.cpp
    src/renderer.cpp
    src/input.cpp
    src/game_loop.cpp
)

target_include_directories(chesspp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(chesspp
    PRIVATE
        chesspp_engine
        chesspp_core
        sfml-graphics
        sfml-window
        sfml-system
)

chesspp_set_warnings(chesspp)
