#!/bin/bash

if [ "$#" -ne 2 ]; then
  echo "Usage: $0 input.png output.car"
  exit 1
fi

INPUT="$1"
OUTPUT="$2"

TMPDIR=$(mktemp -d)
XCASSETS="$TMPDIR/Assets.xcassets"
APPICON="$XCASSETS/OpenEnroth.appiconset"
CANVAS=1024

mkdir -p "$APPICON"

# Create full-bleed 1024x1024 icon with no alpha (Tahoe applies its own mask).
magick -size ${CANVAS}x${CANVAS} xc:black \
  "$INPUT" -filter point -resize ${CANVAS}x${CANVAS} -composite \
  -alpha off \
  "$TMPDIR/icon_1024.png"

# Generate all required sizes.
magick "$TMPDIR/icon_1024.png" -resize 16x16     "$APPICON/icon_16x16.png"
magick "$TMPDIR/icon_1024.png" -resize 32x32     "$APPICON/icon_16x16@2x.png"
magick "$TMPDIR/icon_1024.png" -resize 32x32     "$APPICON/icon_32x32.png"
magick "$TMPDIR/icon_1024.png" -resize 64x64     "$APPICON/icon_32x32@2x.png"
magick "$TMPDIR/icon_1024.png" -resize 128x128   "$APPICON/icon_128x128.png"
magick "$TMPDIR/icon_1024.png" -resize 256x256   "$APPICON/icon_128x128@2x.png"
magick "$TMPDIR/icon_1024.png" -resize 256x256   "$APPICON/icon_256x256.png"
magick "$TMPDIR/icon_1024.png" -resize 512x512   "$APPICON/icon_256x256@2x.png"
magick "$TMPDIR/icon_1024.png" -resize 512x512   "$APPICON/icon_512x512.png"
magick "$TMPDIR/icon_1024.png" -resize 1024x1024 "$APPICON/icon_512x512@2x.png"

# Create Contents.json for the appiconset.
cat > "$APPICON/Contents.json" << 'EOF'
{
  "images" : [
    { "filename" : "icon_16x16.png", "idiom" : "mac", "scale" : "1x", "size" : "16x16" },
    { "filename" : "icon_16x16@2x.png", "idiom" : "mac", "scale" : "2x", "size" : "16x16" },
    { "filename" : "icon_32x32.png", "idiom" : "mac", "scale" : "1x", "size" : "32x32" },
    { "filename" : "icon_32x32@2x.png", "idiom" : "mac", "scale" : "2x", "size" : "32x32" },
    { "filename" : "icon_128x128.png", "idiom" : "mac", "scale" : "1x", "size" : "128x128" },
    { "filename" : "icon_128x128@2x.png", "idiom" : "mac", "scale" : "2x", "size" : "128x128" },
    { "filename" : "icon_256x256.png", "idiom" : "mac", "scale" : "1x", "size" : "256x256" },
    { "filename" : "icon_256x256@2x.png", "idiom" : "mac", "scale" : "2x", "size" : "256x256" },
    { "filename" : "icon_512x512.png", "idiom" : "mac", "scale" : "1x", "size" : "512x512" },
    { "filename" : "icon_512x512@2x.png", "idiom" : "mac", "scale" : "2x", "size" : "512x512" }
  ],
  "info" : { "author" : "xcode", "version" : 1 }
}
EOF

# Create root Contents.json for xcassets.
cat > "$XCASSETS/Contents.json" << 'EOF'
{
  "info" : { "author" : "xcode", "version" : 1 }
}
EOF

# Compile to Assets.car using actool.
xcrun actool "$XCASSETS" \
  --compile "$TMPDIR" \
  --platform macosx \
  --minimum-deployment-target 11.0 \
  --app-icon OpenEnroth \
  --output-partial-info-plist "$TMPDIR/Info.plist" > /dev/null 2>&1

mv "$TMPDIR/Assets.car" "$OUTPUT"

# Clean up.
rm -rf "$TMPDIR"

echo "✅ $OUTPUT"
