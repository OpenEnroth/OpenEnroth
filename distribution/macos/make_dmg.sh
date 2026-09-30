#!/bin/bash

set -euo pipefail

if [[ $# -ne 4 ]]; then
    echo "Usage: $0 OpenEnroth.app volume_icon.icns background.jpg output.dmg"
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
