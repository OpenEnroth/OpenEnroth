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

# The claude binary is an optional platform dependency, so npm exits 0 even when its download fails, and
# leaves a non-executable stub in its place.
if ! devcontainer exec --workspace-folder "$REPO_DIR" bash -lc 'claude --version' &>/dev/null; then
    echo "Claude Code in the container is broken, reinstalling..."
    # npm renames the old tree aside before writing the new one, and that rename fails with ENOTEMPTY on a
    # half-installed package.
    devcontainer exec --workspace-folder "$REPO_DIR" bash -lc \
      'rm -rf "$(npm root -g)/@anthropic-ai/claude-code" && npm install -g @anthropic-ai/claude-code@latest' || true

    if ! devcontainer exec --workspace-folder "$REPO_DIR" bash -lc 'claude --version' &>/dev/null; then
        echo "Error: could not repair Claude Code in the container."
        echo "Rebuild the image with: devcontainer up --build-no-cache --workspace-folder \"$REPO_DIR\""
        exit 1
    fi
fi

# Claude's agent view is the entry point. Sessions dispatched from it keep running when the host terminal
# dies. A dead terminal only takes down the view itself. Rerun this script to get it back, with every
# session still in it.
#
# TERM and COLORTERM are forwarded by hand. Plain `devcontainer exec` passes a bare TERM=xterm and drops
# COLORTERM.
#
# TERM_PROGRAM and TERM_PROGRAM_VERSION go the same way, and they are how Claude names the host terminal.
# Without them it assumes a generic Linux one and says to hold Shift to select text, but Terminal.app needs
# Fn and iTerm2 needs Option.
#
# The agent view turns on mouse reporting, and a terminal that has handed the mouse over stops making
# selections of its own, which leaves Cmd+C nothing to copy. CLAUDE_CODE_DISABLE_MOUSE turns it back off at
# the cost of wheel scrolling.
exec devcontainer exec --workspace-folder "$REPO_DIR" \
  bash -lc "TERM='${TERM:-xterm-256color}' COLORTERM='${COLORTERM:-}' \
            TERM_PROGRAM='${TERM_PROGRAM:-}' TERM_PROGRAM_VERSION='${TERM_PROGRAM_VERSION:-}' \
            CLAUDE_CODE_DISABLE_MOUSE=1 \
            exec claude agents --dangerously-skip-permissions"
