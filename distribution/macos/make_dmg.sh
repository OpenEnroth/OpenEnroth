#!/bin/bash
#
# Packs a macOS app bundle into a dmg that opens as a window with the app, an Applications link to drag it onto, and
# the given background. The window takes the background's size. Needs dmgbuild, `pip install dmgbuild`.
#
# Usage: make_dmg.sh <app> <icon> <background> <output>
#   app         - Path to the app bundle, e.g. build/src/Bin/OpenEnroth/OpenEnroth.app
#   icon        - Path to the .icns file for the volume icon
#   background  - Path to the 1x background png. A <name>@2x.png next to it is used on Retina screens.
#   output      - Path of the dmg to write, overwritten if it exists
#
# Example:
#   ./distribution/macos/make_dmg.sh build/src/Bin/OpenEnroth/OpenEnroth.app src/Bin/OpenEnroth/OpenEnroth.icns \
#       distribution/macos/background.png OpenEnroth.dmg

set -euo pipefail

if [[ $# -lt 4 ]]; then
    echo "Usage: $0 <app> <icon> <background> <output>"
    exit 1
fi

APP="$1"
ICON="$2"
BACKGROUND="$3"
OUTPUT="$4"

python3 -m dmgbuild \
    -s "$(dirname "$0")/dmg_settings.py" \
    -D app="$APP" \
    -D icon="$ICON" \
    -D background="$BACKGROUND" \
    "$(basename "$APP" .app)" \
    "$OUTPUT"
