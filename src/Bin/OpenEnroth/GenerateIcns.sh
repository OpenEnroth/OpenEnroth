#!/bin/bash

if [ "$#" -ne 2 ]; then
  echo "Usage: $0 input.png output.icns"
  exit 1
fi

INPUT="$1"
OUTPUT="$2"

TMPDIR=$(mktemp -d)
ICONSET="$TMPDIR/icon.iconset"
mkdir "$ICONSET"

ICON_SIZE=824
CORNER_RADIUS=185.4
CANVAS=1024
SHADOW_OFFSET_Y=12
SHADOW_BLUR=28
SHADOW_OPACITY=50

# Resize input to 824x824 & paste on top of a black canvas.
# `-filter point` forces nearest neighbor resizing.
magick -size ${ICON_SIZE}x${ICON_SIZE} xc:black \
  "$INPUT" -filter point -resize ${ICON_SIZE}x${ICON_SIZE} -alpha on -composite \
  "$TMPDIR/resized.png"

# Create iOS-style rounded mask.
magick -size ${ICON_SIZE}x${ICON_SIZE} xc:none \
  -draw "roundrectangle 0,0 $((ICON_SIZE-1)),$((ICON_SIZE-1)) $CORNER_RADIUS,$CORNER_RADIUS" \
  "$TMPDIR/mask.png"

# Apply rounded mask.
magick "$TMPDIR/resized.png" "$TMPDIR/mask.png" -compose DstIn -composite "$TMPDIR/rounded.png"

# Create drop shadow (same size as icon).
magick "$TMPDIR/rounded.png" -background none \
  -shadow ${SHADOW_OPACITY}x${SHADOW_BLUR}+0+${SHADOW_OFFSET_Y} "$TMPDIR/shadow.png"

# Place shadow in fixed 1024x1024 canvas, offset downward.
magick -size ${CANVAS}x${CANVAS} xc:none "$TMPDIR/shadow.png" \
  -gravity center -geometry +0+${SHADOW_OFFSET_Y} -composite "$TMPDIR/shadow_layer.png"

# Composite the icon on top, centered.
# xc:none is a background image, fully transparent in this case.
magick -size 1024x1024 xc:none \
  "$TMPDIR/shadow_layer.png" -gravity center -composite \
  "$TMPDIR/rounded.png" -gravity center -composite \
  "$TMPDIR/final.png"

# Generate required iconset sizes.
magick "$TMPDIR/final.png" -resize 16x16     "$ICONSET/icon_16x16.png"
magick "$TMPDIR/final.png" -resize 32x32     "$ICONSET/icon_16x16@2x.png"
magick "$TMPDIR/final.png" -resize 32x32     "$ICONSET/icon_32x32.png"
magick "$TMPDIR/final.png" -resize 64x64     "$ICONSET/icon_32x32@2x.png"
magick "$TMPDIR/final.png" -resize 128x128   "$ICONSET/icon_128x128.png"
magick "$TMPDIR/final.png" -resize 256x256   "$ICONSET/icon_128x128@2x.png"
magick "$TMPDIR/final.png" -resize 256x256   "$ICONSET/icon_256x256.png"
magick "$TMPDIR/final.png" -resize 512x512   "$ICONSET/icon_256x256@2x.png"
magick "$TMPDIR/final.png" -resize 512x512   "$ICONSET/icon_512x512.png"
magick "$TMPDIR/final.png" -resize 1024x1024 "$ICONSET/icon_512x512@2x.png"

# Create .icns.
iconutil --convert icns "$ICONSET" --output "$OUTPUT"

# Clean up.
rm -rf "$TMPDIR"

echo "✅ $OUTPUT"
