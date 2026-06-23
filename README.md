# Chess++

C++20 based chess engine with SFML supported GUI Interface.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/src/app/chesspp
```

## Project layout

```text
chess++/
├── CMakeLists.txt              # Root project, SFML via FetchContent
├── cmake/CompilerWarnings.cmake
├── src/
│   ├── core/                   # Rules, board, move generation
│   │   ├── include/
│   │   │   ├── types.hpp       # Color, Piece, Square, Move
│   │   │   ├── board.hpp       # Board state
│   │   │   ├── movegen.hpp     # Legal move generation
│   │   │   └── game.hpp        # Game flow, draw/checkmate
│   │   └── src/
│   ├── engine/                 # Search and evaluation
│   │   ├── include/
│   │   │   ├── evaluator.hpp
│   │   │   ├── search.hpp
│   │   │   └── engine.hpp
│   │   └── src/
│   └── app/                    # SFML executable
│       ├── include/
│       │   ├── renderer.hpp
│       │   ├── input.hpp
│       │   └── game_loop.hpp
│       └── src/
│           └── main.cpp
```
