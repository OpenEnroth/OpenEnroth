#!/bin/bash
#
# Packs a macOS app bundle into a dmg that opens as a window with the app, an Applications link to drag it onto, and
# the background from distribution/macos. Needs dmgbuild, `pip install dmgbuild`.
#
# Usage: make_dmg.sh <app> <icon> <output>
#   app     - Path to the app bundle, e.g. build/src/Bin/OpenEnroth/OpenEnroth.app
#   icon    - Path to the .icns file for the volume icon
#   output  - Path of the dmg to write, overwritten if it exists
#
# Example:
#   ./scripts/make_dmg.sh build/src/Bin/OpenEnroth/OpenEnroth.app src/Bin/OpenEnroth/OpenEnroth.icns OpenEnroth.dmg

set -euo pipefail

if [[ $# -lt 3 ]]; then
    echo "Usage: $0 <app> <icon> <output>"
    exit 1
fi

APP="$1"
ICON="$2"
OUTPUT="$3"
DMG_DIR="$(cd "$(dirname "$0")/../distribution/macos" && pwd)"

# dmgbuild picks up background@2x.png next to background.png for Retina screens.
python3 -m dmgbuild \
    -s "$DMG_DIR/dmg_settings.py" \
    -D app="$APP" \
    -D icon="$ICON" \
    -D background="$DMG_DIR/background.png" \
    "$(basename "$APP" .app)" \
    "$OUTPUT"
