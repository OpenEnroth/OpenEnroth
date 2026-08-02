#!/bin/bash
set -euo pipefail

REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"

if ! command -v devcontainer &>/dev/null; then
    echo "Error: 'devcontainer' CLI not found."
    echo "Install it with: npm install -g @devcontainers/cli"
    exit 1
fi

if ! docker info &>/dev/null; then
    echo "Error: Docker is not running. Please start Docker and try again."
    exit 1
fi

echo "Starting devcontainer (no-op if already running)..."
devcontainer up --workspace-folder "$REPO_DIR" >/dev/null

# Run Claude inside tmux so it survives disconnects from the host terminal. `-A` attaches to an existing session,
# so this same command both starts and reattaches; `-D` kicks off any stale client so the terminal size is right.
# TERM and COLORTERM are forwarded from the host - `devcontainer exec` passes a bare TERM=xterm and drops
# COLORTERM, which makes tmux quantize colors down to 8.
exec devcontainer exec --workspace-folder "$REPO_DIR" \
  bash -lc "TERM='${TERM:-xterm-256color}' COLORTERM='${COLORTERM:-}' exec tmux new-session -A -D -s claude 'claude --dangerously-skip-permissions; exec bash -l'"
