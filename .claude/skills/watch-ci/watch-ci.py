#!/usr/bin/env python3
"""Waits for GitHub CI on one commit and prints a trustworthy verdict, reporting the first failure as soon as it
lands. See SKILL.md in this directory for the failure modes this design guards against."""
import argparse
import json
import re
import subprocess
import sys
import time
from datetime import datetime, timezone

sys.dont_write_bytecode = True  # Set before the import, so the skill directory never grows a __pycache__.
import verdict

POLL_SECONDS = 60
PER_PAGE = 100
MAX_PAGES = 20  # A commit with more than 2000 check runs is not something this tool needs to judge.
GH_TIMEOUT = 120


def hexsha(text):
    """Rejects anything that is not a commit hash, so a branch name never becomes a silent horizon-long wait.

    @param text                         Argument as typed on the command line.
    @return                             The same text, once it looks like a commit hash.
    """
    if not re.fullmatch(r"[0-9a-fA-F]{7,40}", text):
        raise argparse.ArgumentTypeError(f"{text!r} is not a commit hash, pass the sha explicitly")
    return text


def page(repo, sha, number):
    """Reads one page of check runs.

    @param repo                         Repository in owner/name form.
    @param sha                          Commit to ask about.
    @param number                       Page to read, counting from one.
    @return                             Parsed response, or None when the answer was not JSON at all.
    """
    url = f"repos/{repo}/commits/{sha}/check-runs?per_page={PER_PAGE}&page={number}"
    try:
        done = subprocess.run(["gh", "api", url], capture_output=True, text=True, timeout=GH_TIMEOUT)
    except FileNotFoundError:
        # Polling cannot install gh, and exit 1 would read as a CI failure, so this is the no-verdict code.
        print("RESULT: NO_GH - the check runs can only be read through gh - not a pass/fail verdict")
        sys.exit(2)
    except subprocess.TimeoutExpired:
        return None
    try:
        return json.loads(done.stdout)  # A 4xx body arrives on stdout too, and the verdict reads it.
    except ValueError:
        # gh writes its own diagnostics to stderr, and an unauthenticated one repeats forever.
        return {"message": done.stderr.strip().splitlines()[0] if done.stderr.strip() else "no output from gh",
                "status": "401" if "auth" in done.stderr.lower() else str(done.returncode)}


def fetch(repo, sha):
    """Reads every check run GitHub has for one commit, following pagination to the end.

    @param repo                         Repository in owner/name form.
    @param sha                          Commit to ask about.
    @return                             Every page's check runs in one response, or the error body that came instead.
    """
    runs = []
    total = 0
    for number in range(1, MAX_PAGES + 1):
        payload = page(repo, sha, number)
        if not isinstance(payload, dict) or "check_runs" not in payload:
            return payload
        runs += payload["check_runs"]
        total = payload.get("total_count")
        if not payload["check_runs"] or total is None or len(runs) >= total:
            break
    return {"total_count": total, "check_runs": runs}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("sha", type=hexsha, help="the exact commit to watch")
    parser.add_argument("names", nargs="*", help="check names to wait for, empty to judge the whole run")
    parser.add_argument("-m", "--minutes", type=int, default=150, help="polling horizon, default 150")
    parser.add_argument("-c", "--min-checks", type=int, default=15, help="checks needed for a green verdict")
    parser.add_argument("-r", "--repo", default="OpenEnroth/OpenEnroth", help="owner/name, default OpenEnroth")
    args = parser.parse_intermixed_args()

    scope = ("waiting for: " + " ".join(args.names)) if args.names else f"min checks {args.min_checks}"
    print(f"watching {args.sha} on {args.repo} (horizon {args.minutes}m, {scope})", flush=True)

    deadline = time.monotonic() + args.minutes * 60
    last = []
    while True:
        code, lines = verdict.decide(fetch(args.repo, args.sha), args.min_checks, args.names)
        if code != verdict.WAIT:
            print("\n".join(lines), flush=True)
            return code
        if lines != last:
            print(f"{datetime.now(timezone.utc):%H:%M:%SZ} {lines[-1]}", flush=True)
            last = lines
        if time.monotonic() >= deadline:
            break
        time.sleep(POLL_SECONDS)

    assert last, "the loop polls at least once, and every WAIT it can return carries a line"
    print(f"RESULT: TIMED_OUT - {last[-1].removeprefix('WAIT ')} - not a pass/fail verdict", flush=True)
    return 2


if __name__ == "__main__":
    try:
        sys.exit(main())
    except Exception as e:
        print(f"RESULT: CRASHED - {type(e).__name__}: {e} - not a pass/fail verdict")
        sys.exit(2)
