#!/bin/bash

set -e

if [[ $# -lt 2 ]]; then
    echo "Usage: $0 shader.frag \"410 core\""
    exit 1
fi

SHADER="$1"
VERSION="$2"
SHADER_DIR="$(dirname "$SHADER")"
SHADER_NAME="$(basename "$SHADER")"

TEMP_DIR=$(mktemp -d)
trap "rm -rf $TEMP_DIR" EXIT

# Build preamble
PREAMBLE="#version $VERSION
#extension GL_GOOGLE_include_directive : enable
"

# Create temp file with preamble
echo -n "$PREAMBLE" > "$TEMP_DIR/$SHADER_NAME"
cat "$SHADER" >> "$TEMP_DIR/$SHADER_NAME"

# Preprocess to expand includes
if ! glslangValidator -I"$SHADER_DIR" -E "$TEMP_DIR/$SHADER_NAME" > "$TEMP_DIR/pp_$SHADER_NAME" 2>&1; then
    echo "Preprocessing failed:"
    cat "$TEMP_DIR/pp_$SHADER_NAME"
    exit 1
fi

# Validate
glslangValidator "$TEMP_DIR/pp_$SHADER_NAME"
