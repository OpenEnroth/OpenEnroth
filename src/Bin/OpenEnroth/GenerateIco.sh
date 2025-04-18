#!/bin/bash

if [ "$#" -ne 2 ]; then
  echo "Usage: $0 input.png output.ico"
  exit 1
fi

INPUT="$1"
OUTPUT="$2"

magick "$INPUT" -define icon:auto-resize="256,128,96,64,48,32,16" "$OUTPUT"

echo "✅ $OUTPUT"
