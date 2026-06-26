cmake_minimum_required(VERSION 3.20)

project(chesspp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerWarnings)

include(FetchContent)

set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    sfml
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        3.1.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(sfml)

add_subdirectory(src/core)
add_subdirectory(src/engine)
add_subdirectory(src/app)

option(CHESSPP_BUILD_TESTS "Build chess++ unit tests" ON)
if(CHESSPP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
