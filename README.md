# ♟️ FlameBot Chess

A desktop chess application with a premium **Electron** GUI and a powerful **C++ chess engine**.

> **Engine credit:** The chess engine is based on [TerminalChess](https://github.com/superroket169/TerminalChess) by [@superroket169](https://github.com/superroket169).

---

## ✨ Features

### 🎮 GUI
- **Premium dark UI** with glassmorphism, gradients, and micro-animations
- **6 themes:** Obsidian, Emerald, Ocean, Crimson, Arctic, Neon
- **3 game modes:** Human vs Human, Human vs Bot, Bot vs Bot
- **Game history** with full replay (forward/backward/auto-play)
- **Move notation** with algebraic display
- **Tabbed interface** — Game view & Settings panel
- **Responsive layout** adapts to any window size

### ♚ Engine (FlameBot)
- **Minimax** with **Alpha-Beta Pruning**
- **Transposition Table** using Zobrist hashing
- **Move Ordering** — captures & checks prioritized
- **Iterative Deepening** with time management
- **Opening Book** support
- **UCI protocol** for standard GUI-engine communication

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
├── gui/                    # Electron GUI
│   ├── main.js             # Main process + engine spawn
│   ├── preload.js          # Secure IPC bridge
│   ├── index.html          # Tabbed layout
│   ├── css/style.css       # 6-theme design system
│   └── js/
│       ├── pieces.js       # SVG piece definitions
│       ├── game.js         # Full chess rule engine
│       ├── board.js        # Board rendering & interaction
│       └── app.js          # App logic, themes, replay
├── CMakeLists.txt          # Engine build config
└── README.md
```

---

## 🛠️ Build & Run

### Prerequisites
- **C++ Compiler:** GCC, Clang, or MinGW (C++17)
- **CMake:** 3.21+
- **Node.js:** 18+ with npm

### 1. Build the Engine

```bash
cmake -S . -B build
cmake --build build
```

This produces `build/FlameBot.exe` (Windows) or `build/FlameBot` (Linux/Mac).

### 2. Run the GUI

```bash
cd gui
npm install
npm start
```

The GUI will automatically detect and connect to the engine at `build/FlameBot.exe`.

### 3. Package as Portable App (Windows)

```bash
cd gui
npm run package
```

Output: `gui/dist/FlameBot-Chess-win32-x64/` — copy the entire folder to a USB drive and run `FlameBot-Chess.exe` on any Windows PC, no installation needed.

---

## 🎨 Themes

| Theme | Style |
|-------|-------|
| **Obsidian** | Purple accent, dark board |
| **Emerald** | Green accent, Lichess-style board |
| **Ocean** | Blue accent, sea tones |
| **Crimson** | Red accent, warm tones |
| **Arctic** | Light theme, classic board |
| **Neon** | Pink/purple accent, cyberpunk |

---

## 🔧 Architecture

```
┌──────────────┐    IPC     ┌───────────────┐  stdin/stdout  ┌──────────────┐
│  Renderer    │◄──────────►│  Main Process │◄──────────────►│  FlameBot    │
│  (app.js)    │            │  (main.js)    │   UCI protocol │  (C++ engine)│
└──────────────┘            └───────────────┘                └──────────────┘
```

The GUI communicates with the engine via the **UCI protocol** over stdin/stdout. The Electron main process spawns the engine as a child process and bridges messages through IPC.

---

## 📜 License

This project is licensed under the **GNU General Public License v3.0** — see the [LICENSE](LICENSE) file for details.

---

## 🙏 Credits

- **Engine:** [TerminalChess](https://github.com/superroket169/TerminalChess) by [@superroket169](https://github.com/superroket169)
- **GUI Framework:** [Electron](https://www.electronjs.org/)
- **Font:** [Inter](https://rsms.me/inter/) by Rasmus Andersson