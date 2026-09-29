#!/bin/bash
#
# Packs a macOS app bundle into a compressed dmg that opens with the app next to an Applications link, and shows the
# given icon as its volume icon.
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
VOLUME_NAME="$(basename "$APP" .app)"

WORK_DIR="$(mktemp -d)"
STAGING_DIR="$WORK_DIR/staging"
STAGING_IMAGE="$WORK_DIR/staging.dmg"
MOUNT_POINT=""

cleanup() {
    if [[ -n "$MOUNT_POINT" ]]; then
        hdiutil detach "$MOUNT_POINT" -force > /dev/null || true
    fi
    rm -rf "$WORK_DIR"
}
trap cleanup EXIT

mkdir "$STAGING_DIR"
cp -R "$APP" "$STAGING_DIR/"
ln -s /Applications "$STAGING_DIR/Applications"
cp "$ICON" "$STAGING_DIR/.VolumeIcon.icns"

# The custom icon flag can only be set on a mounted writable image, so this one is converted afterwards.
hdiutil create "$STAGING_IMAGE" -volname "$VOLUME_NAME" -fs HFS+ -format UDRW -srcfolder "$STAGING_DIR"
MOUNT_POINT="$(hdiutil attach "$STAGING_IMAGE" -nobrowse -noautoopen | awk -F '\t' '/Apple_HFS/ {print $NF}')"
SetFile -a C "$MOUNT_POINT" # Finder only shows .VolumeIcon.icns when the volume root has the custom icon flag.
hdiutil detach "$MOUNT_POINT"
MOUNT_POINT=""

hdiutil convert "$STAGING_IMAGE" -ov -format UDZO -o "$OUTPUT"
