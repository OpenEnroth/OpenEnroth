#!/usr/bin/env bash
# Waits for GitHub CI on one commit and prints a trustworthy verdict, reporting the first failure as soon as it
# lands. See SKILL.md in this directory for the failure modes this design guards against.
set -u

usage() {
    echo "usage: watch-ci.sh <sha> [-m minutes] [-c min_checks] [-r repo] [check-name ...]" >&2
    exit 2
}

SHA="${1:-}"
[ -n "$SHA" ] || usage
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
NAMES_JSON=$(python3 -c 'import json, sys; print(json.dumps(sys.argv[1:]))' "$@")

echo "watching $SHA on $REPO (horizon ${MINUTES}m, min checks $MIN_CHECKS, named: $NAMES_JSON)"

# Exit 3 from the verdict means keep waiting, anything else is the final answer.
verdict() {
    WATCH_NAMES="$NAMES_JSON" WATCH_MIN_CHECKS="$MIN_CHECKS" python3 -c '
import json, os, sys
from collections import Counter

names = json.loads(os.environ["WATCH_NAMES"])
min_checks = int(os.environ["WATCH_MIN_CHECKS"])
try:
    runs = json.load(sys.stdin).get("check_runs", [])
except Exception:
    print("WAIT api error")
    sys.exit(3)

BAD = ("failure", "timed_out", "action_required", "startup_failure")
NO_VERDICT = ("cancelled", "stale")

def describe(run):
    where = f" {run['"'"'html_url'"'"']}" if run["conclusion"] in BAD else ""
    return f"{run['"'"'name'"'"']}: {run['"'"'conclusion'"'"'] or run['"'"'status'"'"']}{where}"

if names:
    present = {r["name"]: r for r in runs}
    missing = [n for n in names if n not in present]
    named = [present[n] for n in names if n in present]
    pending = [r for r in named if r["status"] != "completed"]
    if missing or pending:
        detail = f"named {len(named) - len(pending)}/{len(names)} completed"
        if missing:
            detail += ", not started: " + ", ".join(missing)
        print("WAIT " + detail)
        sys.exit(3)
    for r in named:
        print(describe(r))
    bad = [r["name"] for r in named if r["conclusion"] in BAD]
    if bad:
        print("RESULT: FAILING -> " + ", ".join(bad))
        sys.exit(1)
    cancelled = [r["name"] for r in named if r["conclusion"] in NO_VERDICT]
    if cancelled:
        print("RESULT: CANCELLED -> " + ", ".join(cancelled) + " - not a pass/fail verdict")
        sys.exit(2)
    print("RESULT: ALL GREEN")
    sys.exit(0)

# A failure is final the moment it lands, whatever the rest is doing. Cancelled checks carry no verdict, but a
# failure elsewhere in the same run is still a failure, and reporting CANCELLED would bury it.
bad = [r for r in runs if r["status"] == "completed" and r["conclusion"] in BAD]
if bad:
    for r in bad:
        print(describe(r))
    still = sum(1 for r in runs if r["status"] != "completed")
    print("RESULT: FAILING -> " + ", ".join(r["name"] for r in bad) + f" ({still} still running)")
    sys.exit(1)

states = Counter(r["status"] for r in runs)
if len(runs) < min_checks or states.get("completed", 0) < len(runs):
    print(f"WAIT {states.get('"'"'completed'"'"', 0)}/{len(runs)} completed, min {min_checks}")
    sys.exit(3)

cancelled = [r["name"] for r in runs if r["conclusion"] in NO_VERDICT]
if cancelled:
    print("RESULT: CANCELLED -> " + ", ".join(cancelled) + " - not a pass/fail verdict")
    sys.exit(2)
print("RESULT: ALL GREEN")
sys.exit(0)
'
}

last=""
for _ in $(seq 1 "$MINUTES"); do
    json=$(gh api "repos/$REPO/commits/$SHA/check-runs?per_page=100" 2>/dev/null)
    out=$(printf '%s' "$json" | verdict)
    code=$?
    if [ "$code" -ne 3 ]; then
        printf '%s\n' "$out"
        exit "$code"
    fi
    if [ "$out" != "$last" ]; then
        echo "$(date -u +%H:%M:%SZ) $out"
        last="$out"
    fi
    sleep 60
done

echo "RESULT: TIMED_OUT - ${last#WAIT } - not a pass/fail verdict"
exit 2
