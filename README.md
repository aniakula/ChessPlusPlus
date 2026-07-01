# Chess++

A C++20 chess program with a bitboard-based rules core, an alpha-beta search engine (in development), and an SFML desktop GUI.

## Build and Run

### Requirements

- CMake 3.20 or newer
- A C++20 compiler (Clang, GCC, or MSVC)
- Git (SFML is fetched automatically on first configure)

### Quick start

From the repository root:

```bash
chmod +x chess++
./chess++
```

The `chess++` script configures CMake on the first run, builds the `chesspp` target, and launches the game. Run it again to rebuild only what changed and start a new session.

Optional: add the repo to your `PATH` so `chess++` works from anywhere:

```bash
export PATH="/path/to/chess++:$PATH"
chess++
```

### Manual build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target chesspp -j
./build/src/app/chesspp
```

Run the binary from the repository root so asset paths (`src/assets/...`) resolve correctly.

### Tests

```bash
cmake --build build --target board_tests -j
cd build && ctest --output-on-failure
```

---

## How to Play

You play against the engine.

1. **Click a piece** on your turn. The selected square is highlighted in gold; legal destination squares are highlighted in blue.
2. **Click a highlighted square** to play that move.
3. The engine replies automatically on its turn.

Game status and future dialogs appear in the left panel; engine output will stream to the right panel.

---

## Technical Overview

### Architecture

- **`src/core`** — Board, movegen, legality, game rules, FEN/PGN, Zobrist hashing
- **`src/engine`** — Search and evaluation; takes a `Board`, returns `SearchResult`
- **`src/app`** — SFML GUI: input, rendering, game loop; human via `Game`, engine via `Engine`

### Board representation

- Twelve **bitboards** (one per color/piece type) plus combined occupancy masks
- Game state: side to move, castling, en passant, halfmove/fullmove clocks, Zobrist `HashKey`
- Squares: `uint8_t` 0–63; `Move`: 32-bit packed from/to/promotion/flags
- `make_move` / `unmake_move` via `UndoState` — no heap allocation in search
- Pseudo-legal gen → `leaves_king_safe` filter
- `Game` adds repetition history and result detection (checkmate, stalemate, draws)

### Move generation

- `MoveList` — fixed stack buffer (max 256 moves)
- `generate_pseudo_legal` / `generate_legal` / per-square overload
- Precomputed attacks for knights, kings, pawns; ray walks for sliders

### GUI (`src/app`)

- Window **1360×800**: left panel (280) | board (800) | right panel (280)
- `ui_layout` offsets mouse coords so clicks on panels do not select squares
- `InputHandler` → `InputAction`; holds selection + legal moves between frames
- `Renderer` — draw only; `GameLoop` — events, moves, engine turn
- Left panel: status, dialogs (stub); right panel: engine log (stub)

### Engine *(in development)*

**Target search flow:**

- `Engine::set_position` → `think` under `SearchLimits` (depth / nodes / time)
- Iterative deepening → `alpha_beta` (negamax + pruning)
- Depth 0 → quiescence (captures/promotions)
- `Evaluator` — material, PST, later mobility/king safety
- `MoveOrderer` — TT move, captures, killers, history
- `TranspositionTable` — Zobrist-indexed buckets; probe/store implemented

**Per-node loop (planned):**

- TT probe → quiescence or terminal score → gen legal moves → order → make/recurse/unmake → TT store

**Current:** `Engine::think()` plays a **random legal move**. Search and eval bodies are stubs.

**Planned:**

- Alpha-beta + quiescence, iterative deepening, full eval, move ordering
- UCI, promotion/game-end UI, engine log panel, opening book, time management

### Testing & tooling

- GoogleTest: board, FEN, movegen, game rules (104 tests)
- CMake + FetchContent for SFML 3.1; C++20; `compile_commands.json` for clangd
