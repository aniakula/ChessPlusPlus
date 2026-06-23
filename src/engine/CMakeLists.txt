add_library(chesspp_engine STATIC
    src/evaluator.cpp
    src/search.cpp
    src/engine.cpp
)

target_include_directories(chesspp_engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(chesspp_engine
    PUBLIC
        chesspp_core
)

chesspp_set_warnings(chesspp_engine)
