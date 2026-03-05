#!/bin/bash
set -euo pipefail

REPO_DIR="$(cd "$(dirname "$0")" && pwd)"

if ! command -v devcontainer &>/dev/null; then
    echo "Error: 'devcontainer' CLI not found."
    echo "Install it with: npm install -g @devcontainers/cli"
    exit 1
fi

echo "Starting devcontainer (no-op if already running)..."
devcontainer up --workspace-folder "$REPO_DIR" >/dev/null

exec devcontainer exec --workspace-folder "$REPO_DIR" \
  bash -lc 'claude --dangerously-skip-permissions; exec bash -l'
