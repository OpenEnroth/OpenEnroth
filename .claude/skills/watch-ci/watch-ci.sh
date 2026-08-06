#!/usr/bin/env bash
# Waits for GitHub CI on one commit and prints a trustworthy three-way verdict.
# See SKILL.md in this directory for the failure modes this design guards against.
set -u

SHA="${1:?usage: watch-ci.sh <sha> [minutes] [min_checks] [repo]}"
MINUTES="${2:-150}"
MIN_CHECKS="${3:-15}"
REPO="${4:-OpenEnroth/OpenEnroth}"

echo "watching $SHA on $REPO (horizon ${MINUTES}m, min checks $MIN_CHECKS)"

json=""
for _ in $(seq 1 "$MINUTES"); do
    json=$(gh api "repos/$REPO/commits/$SHA/check-runs?per_page=100" 2>/dev/null)
    state=$(printf '%s' "$json" | python3 -c "
import sys, json
try:
    rs = json.load(sys.stdin).get('check_runs', [])
except Exception:
    print('RETRY')
    raise SystemExit
print('DONE' if len(rs) >= $MIN_CHECKS and all(r['status'] == 'completed' for r in rs) else 'RETRY')
" 2>/dev/null)
    [ "$state" = "DONE" ] && break
    sleep 60
done

printf '%s' "$json" | python3 -c "
import sys, json
from collections import Counter

try:
    rs = json.load(sys.stdin).get('check_runs', [])
except Exception:
    rs = []

states = Counter(r['status'] for r in rs)
print('checks:', len(rs), '| states:', dict(states))

if len(rs) < $MIN_CHECKS or states.get('completed', 0) < len(rs):
    print('RESULT: TIMED_OUT - not a pass/fail verdict')
    raise SystemExit(2)

# A real failure outranks a cancellation - cancelled checks carry no verdict, but a failure elsewhere in the same
# run is still a failure, and reporting CANCELLED would bury it.
bad = [r['name'] for r in rs if r['conclusion'] not in ('success', 'cancelled')]
if bad:
    print('RESULT: FAILING ->', ', '.join(bad))
    raise SystemExit(1)

cancelled = [r['name'] for r in rs if r['conclusion'] == 'cancelled']
if cancelled:
    print('RESULT: CANCELLED -> ' + ', '.join(cancelled) + ' - not a pass/fail verdict')
    raise SystemExit(2)

print('RESULT: ALL GREEN')
"
exit $?
