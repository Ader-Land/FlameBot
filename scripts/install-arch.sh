#!/usr/bin/env bash
set -euo pipefail

SOURCE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP_DIR="/opt/flamebot-chess"
BIN_LINK="/usr/local/bin/chess"
DESKTOP_FILE="/usr/share/applications/flamebot-chess.desktop"

if [[ "${EUID:-$(id -u)}" -ne 0 ]]; then
  echo "Run this installer with sudo:"
  echo "  sudo ./install.sh"
  exit 1
fi

rm -rf "$APP_DIR"
mkdir -p "$APP_DIR"
cp -R "$SOURCE_DIR"/. "$APP_DIR"/

if [[ -f "$APP_DIR/chess" ]]; then
  chmod +x "$APP_DIR/chess"
fi

ln -sfn "$APP_DIR/chess" "$BIN_LINK"

cat > "$DESKTOP_FILE" <<EOF
[Desktop Entry]
Type=Application
Name=FlameBot Chess
Exec=chess
Icon=applications-games
Terminal=false
Categories=Game;BoardGame;
EOF

echo "Installed FlameBot Chess."
echo "Start it with: chess"
