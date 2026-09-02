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


def commit(text):
    """Rejects anything that is not a commit hash, so a branch name never becomes a silent horizon-long wait.

    @param text                         Argument as typed on the command line.
    @return                             The same text, once it looks like a commit hash.
    """
    if not re.fullmatch(r"[0-9a-fA-F]{7,40}", text):
        raise argparse.ArgumentTypeError(f"{text!r} is not a commit hash, pass the sha explicitly")
    return text


def fetch(repo, sha):
    """Reads the check runs GitHub currently has for one commit.

    @param repo                         Repository in owner/name form.
    @param sha                          Commit to ask about.
    @return                             Parsed response, or None when the answer was not JSON at all.
    """
    url = f"repos/{repo}/commits/{sha}/check-runs?per_page=100"
    try:
        done = subprocess.run(["gh", "api", url], capture_output=True, text=True)
    except FileNotFoundError:
        return None
    try:
        return json.loads(done.stdout)  # A 4xx body arrives on stdout too, and the verdict reads it.
    except ValueError:
        return None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("sha", type=commit, help="the exact commit to watch")
    parser.add_argument("names", nargs="*", help="check names to wait for, empty to judge the whole run")
    parser.add_argument("-m", "--minutes", type=int, default=150, help="polling horizon, default 150")
    parser.add_argument("-c", "--min-checks", type=int, default=15, help="checks needed for a green verdict")
    parser.add_argument("-r", "--repo", default="OpenEnroth/OpenEnroth", help="owner/name, default OpenEnroth")
    args = parser.parse_intermixed_args()

    scope = "waiting for: " + " ".join(args.names) if args.names else f"min checks {args.min_checks}"
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

    detail = last[-1].removeprefix("WAIT ") if last else "nothing seen"
    print(f"RESULT: TIMED_OUT - {detail} - not a pass/fail verdict")
    return 2


if __name__ == "__main__":
    sys.exit(main())
