#!/bin/sh

# Feeds bazel build stamping, wired up via --workspace_status_command in
# .bazelrc. STABLE_ keys land in stable-status.txt and re-stamp on change.
# The consumer is the stamp genrule in //src/Library/BuildInfo.
# Never fail - a build without git still builds, just unstamped.

echo "STABLE_MM_GIT_REVISION $(git log -1 --format=%h 2>/dev/null || echo unknown)"
echo "STABLE_OE_VERSION $(git describe --always --tags --dirty 2>/dev/null || echo unknown)"
exit 0
