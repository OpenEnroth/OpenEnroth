#!/usr/bin/env python3
"""Turns one check-runs API response on stdin into a CI verdict. Usage: verdict.py MIN_CHECKS [check-name ...]

Exit code 0 is all green, 1 is failing, 2 is no verdict to report, and 3 means poll again."""
import json
import sys

WAIT = 3
BAD = ("failure", "timed_out", "action_required", "startup_failure")
NO_VERDICT = ("cancelled", "stale")


def summarize(names):
    """Joins check names for the RESULT line, keeping it readable when a whole matrix leg went red.

    @param names                        Failing check names, already printed in full above with their URLs.
    @return                             The first few names, with a count standing in for the rest.
    """
    if len(names) <= 3:
        return ", ".join(names)
    return ", ".join(names[:3]) + f" and {len(names) - 3} more"


def describe(run):
    """Formats one check run for the log, with a job URL on the ones worth opening.

    @param run                          Check run object from the API.
    @return                             Name and outcome, plus the job URL if the run failed.
    """
    where = " " + run["html_url"] if run["conclusion"] in BAD else ""
    return f'{run["name"]}: {run["conclusion"] or run["status"]}{where}'


def named_verdict(names, runs, run_over):
    """Reports the named checks once they have all completed, ignoring whatever the rest of the run is doing.

    @param names                        Check names asked for on the command line.
    @param runs                         Check runs returned for the commit.
    @param run_over                     Whether every check on the commit has completed.
    @return                             Exit code, WAIT while any named check is still outstanding.
    """
    present = {r["name"]: r for r in runs}
    missing = [n for n in names if n not in present]
    named = [present[n] for n in names if n in present]
    pending = [r for r in named if r["status"] != "completed"]

    # A name that is still absent once the run is over is a typo, and waiting out the horizon would not find it.
    if missing and run_over:
        for r in named:
            print(describe(r))
        print("RESULT: NOT_FOUND -> " + ", ".join(missing) + " - never ran, check the names")
        return 2
    if missing or pending:
        detail = f"named {len(named) - len(pending)}/{len(names)} completed"
        if missing:
            detail += ", not started: " + ", ".join(missing)
        print("WAIT " + detail)
        return WAIT

    for r in named:
        print(describe(r))
    bad = [r["name"] for r in named if r["conclusion"] in BAD]
    if bad:
        print("RESULT: FAILING -> " + summarize(bad))
        return 1
    cancelled = [r["name"] for r in named if r["conclusion"] in NO_VERDICT]
    if cancelled:
        print("RESULT: CANCELLED -> " + ", ".join(cancelled) + " - not a pass/fail verdict")
        return 2
    print("RESULT: ALL GREEN")
    return 0


def main():
    min_checks = int(sys.argv[1])
    names = sys.argv[2:]

    try:
        payload = json.load(sys.stdin)
        runs = payload["check_runs"]
    except Exception:
        print("WAIT api error")
        return WAIT

    # One page is fetched, so a larger total means checks are missing here and no count below can be trusted.
    truncated = payload.get("total_count", len(runs)) > len(runs)
    completed = [r for r in runs if r["status"] == "completed"]
    # Every check that exists has finished, so no further check run will appear for this commit.
    run_over = not truncated and len(runs) >= min_checks and len(completed) == len(runs)

    if names:
        return named_verdict(names, runs, run_over)

    # A failure is final the moment it lands, whatever the rest is doing. Cancelled checks carry no verdict, but
    # a failure elsewhere in the same run is still a failure, and reporting CANCELLED would bury it.
    bad = [r for r in completed if r["conclusion"] in BAD]
    if bad:
        for r in bad:
            print(describe(r))
        still = len(runs) - len(completed)
        rest = f" ({still} still running)" if still else ""
        print("RESULT: FAILING -> " + summarize([r["name"] for r in bad]) + rest)
        return 1

    if truncated:
        print(f"RESULT: TRUNCATED - {payload['total_count']} checks exist, {len(runs)} returned"
              " - not a pass/fail verdict")
        return 2
    if len(runs) < min_checks or len(completed) < len(runs):
        print(f"WAIT {len(completed)}/{len(runs)} completed, min {min_checks}")
        return WAIT

    cancelled = [r["name"] for r in runs if r["conclusion"] in NO_VERDICT]
    if cancelled:
        print("RESULT: CANCELLED -> " + ", ".join(cancelled) + " - not a pass/fail verdict")
        return 2
    print("RESULT: ALL GREEN")
    return 0


if __name__ == "__main__":
    sys.exit(main())
