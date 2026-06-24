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
)

target_link_libraries(board_tests
    PRIVATE
        chesspp_core
        GTest::gtest_main
)

chesspp_set_warnings(board_tests)

include(GoogleTest)
gtest_discover_tests(board_tests)
