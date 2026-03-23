#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="/opt/secsgem-codex"
FRONTEND_DIR="$PROJECT_DIR/frontend"
BACKEND_DIR="$PROJECT_DIR/backend"
WEB_ROOT="/var/www/secsgem-console"

cd "$PROJECT_DIR"
git pull

cd "$FRONTEND_DIR"
npm install
npm run build

cd "$BACKEND_DIR"
cmake -S . -B build
cmake --build build -j

sudo mkdir -p "$WEB_ROOT"
sudo mkdir -p "$WEB_ROOT/docs"
sudo rm -rf "$WEB_ROOT/dist"
sudo cp -r "$FRONTEND_DIR/dist" "$WEB_ROOT/dist"
sudo cp "$PROJECT_DIR/document/design-overview.html" "$WEB_ROOT/docs/design-overview.html"
sudo cp "$PROJECT_DIR/document/todo-board.html" "$WEB_ROOT/docs/todo-board.html"

if systemctl list-unit-files | grep -q "^secsgem-backend.service"; then
  sudo systemctl restart secsgem-backend
fi

sudo systemctl restart nginx

echo "Deployment finished."
