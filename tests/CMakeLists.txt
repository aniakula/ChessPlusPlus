include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
    GIT_SHALLOW    TRUE
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(board_tests
    board_test.cpp
    pgn_test.cpp
    search_test.cpp
    movelist_test.cpp
    movegen_test.cpp
    game_test.cpp
)

target_include_directories(board_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/engine/include
)

target_link_libraries(board_tests
    PRIVATE
        chesspp_engine
        chesspp_core
        GTest::gtest_main
)

chesspp_set_warnings(board_tests)

include(GoogleTest)
gtest_discover_tests(board_tests
    DISCOVERY_MODE PRE_TEST
)
