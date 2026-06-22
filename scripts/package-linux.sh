#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build"
GUI_DIR="$ROOT_DIR/gui"
PACKAGE_DIR="$GUI_DIR/dist/FlameBot-Chess-linux-x64"

cmake -S "$ROOT_DIR" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR"

cd "$GUI_DIR"
npm install
npm run package:linux

cp "$ROOT_DIR/scripts/install-arch.sh" "$PACKAGE_DIR/install.sh"
chmod +x "$PACKAGE_DIR/install.sh"
