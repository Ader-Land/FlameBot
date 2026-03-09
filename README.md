# ♟️ FlameBot Chess Engine

A powerful **C++ chess engine** with UCI protocol support.

> **Engine credit:** The chess engine is based on [TerminalChess](https://github.com/superroket169/TerminalChess) by [@superroket169](https://github.com/superroket169).

---

## ✨ Features

- **Minimax** with **Alpha-Beta Pruning**
- **Transposition Table** using Zobrist hashing
- **Move Ordering** — captures & checks prioritized
- **Iterative Deepening** with time management
- **Opening Book** support
- **UCI protocol** for standard GUI communication

---

## 📁 Project Structure

```
Chess/
├── src/                    # C++ engine source
│   ├── main.cpp            # UCI protocol entry point
│   ├── Chess/              # Board, moves, rules
│   ├── FlameBoth/          # Search algorithm (minimax + AB)
│   ├── BoardHash/          # Zobrist hashing
│   ├── OpeningBook/        # Opening book reader
│   └── Time/               # Time management
├── CMakeLists.txt          # Engine build config
└── README.md
```

---

## 🛠️ Build & Run

### Prerequisites
- **C++ Compiler:** GCC, Clang, or MinGW (C++17)
- **CMake:** 3.21+

### Build the Engine

```bash
cmake -S . -B build
cmake --build build
```

This produces `build/FlameBot.exe` (Windows) or `build/FlameBot` (Linux/Mac).

### Running

The engine follows the **UCI (Universal Chess Interface)** protocol. You can run it directly and type commands like `uci`, `isready`, `position startpos`, and `go depth 10`.

---

## 📜 License

This project is licensed under the **GNU General Public License v3.0** — see the [LICENSE](LICENSE) file for details.

---

## 🙏 Credits

- **Engine:** [TerminalChess](https://github.com/superroket169/TerminalChess) by [@superroket169](https://github.com/superroket169)