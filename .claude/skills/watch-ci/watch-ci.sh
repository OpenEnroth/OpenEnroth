#!/usr/bin/env bash
# Waits for GitHub CI on one commit and prints a trustworthy verdict, reporting the first failure as soon as it
# lands. The verdict itself lives in verdict.py, see SKILL.md for the failure modes this design guards against.
set -u

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

usage() {
    echo "usage: watch-ci.sh <sha> [-m minutes] [-c min_checks] [-r repo] [check-name ...]" >&2
    exit 2
}

SHA="${1:-}"
# Flags are read after the sha, so without this a leading flag becomes the commit and polls out the horizon.
[[ "$SHA" =~ ^[0-9a-fA-F]{7,40}$ ]] || usage
shift
MINUTES=150
MIN_CHECKS=15
REPO="OpenEnroth/OpenEnroth"
while getopts "m:c:r:" opt; do
    case "$opt" in
        m) MINUTES="$OPTARG" ;;
        c) MIN_CHECKS="$OPTARG" ;;
        r) REPO="$OPTARG" ;;
        *) usage ;;
    esac
done
shift $((OPTIND - 1))

if [ $# -gt 0 ]; then
    echo "watching $SHA on $REPO (horizon ${MINUTES}m, waiting for: $*)"
else
    echo "watching $SHA on $REPO (horizon ${MINUTES}m, min checks $MIN_CHECKS)"
fi

deadline=$((SECONDS + MINUTES * 60))
last=""
while true; do
    json=$(gh api "repos/$REPO/commits/$SHA/check-runs?per_page=100" 2>/dev/null)
    out=$(printf '%s' "$json" | python3 "$HERE/verdict.py" "$MIN_CHECKS" "$@")
    code=$?
    if [ "$code" -ne 3 ]; then
        printf '%s\n' "$out"
        exit "$code"
    fi
    if [ "$out" != "$last" ]; then
        echo "$(date -u +%H:%M:%SZ) $out"
        last="$out"
    fi
    [ "$SECONDS" -lt "$deadline" ] || break
    sleep 60
done

echo "RESULT: TIMED_OUT - ${last#WAIT } - not a pass/fail verdict"
exit 2
