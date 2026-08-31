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

# Claude's agent view is the entry point. Sessions dispatched from it keep running when the host terminal
# dies. A dead terminal only takes down the view itself. Rerun this script to get it back, with every
# session still in it.
#
# TERM and COLORTERM are forwarded by hand. Plain `devcontainer exec` passes a bare TERM=xterm and drops
# COLORTERM.
exec devcontainer exec --workspace-folder "$REPO_DIR" \
  bash -lc "TERM='${TERM:-xterm-256color}' COLORTERM='${COLORTERM:-}' exec claude agents --dangerously-skip-permissions"
