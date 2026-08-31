#!/bin/sh

# Zips the contents of the current directory into the archive given as $1.
# Zip preserves the unix exec bits the released binaries need. The windows
# runners' git-bash has no zip binary, so they fall back to the preinstalled
# 7z, where exec bits don't matter.
if command -v zip >/dev/null 2>&1; then
    exec zip -r -q "$1" .
else
    exec 7z a -bd "$1" .
fi
