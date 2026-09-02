#!/usr/bin/env python3
"""Feeds canned API responses to verdict.py and checks the verdict. Run it after touching either file."""
import json
import os
import subprocess
import sys

VERDICT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "verdict.py")
URL = "https://github.com/o/r/actions/runs/1/job/2"


def check(name, status, conclusion):
    return {"name": name, "status": status, "conclusion": conclusion, "html_url": URL}


def green(count, first=0):
    return [check(f"job{i}", "completed", "success") for i in range(first, first + count)]


def run(runs=(), names=(), min_checks=15, total_count=None, body=None):
    """Runs verdict.py once and returns its exit code with the last line it printed.

    @param runs                         Check runs to put in the response.
    @param names                        Check names to pass on the command line.
    @param min_checks                   Minimum check runs before a green verdict.
    @param total_count                  Reported total, defaults to the number of runs.
    @param body                         Raw stdin, overriding every other field.
    @return                             Tuple of exit code and last printed line.
    """
    if body is None:
        body = json.dumps({"total_count": len(runs) if total_count is None else total_count, "check_runs": list(runs)})
    argv = [sys.executable, VERDICT, str(min_checks)] + list(names)
    done = subprocess.run(argv, input=body, capture_output=True, text=True)
    return done.returncode, done.stdout.strip()


CASES = []


def case(label, expected_code, expected_prefix, contains=None, **kwargs):
    CASES.append((label, expected_code, expected_prefix, contains, kwargs))


# A transient empty response once made "no pending checks" vacuously true and produced "FAILING: none".
case("empty response", 3, "WAIT", runs=[])
case("malformed body", 3, "WAIT api error", body="<html>502</html>")
# A watcher once timed out with every check still queued and read conclusion null as a failure.
case("all queued", 3, "WAIT", runs=[check(f"job{i}", "queued", None) for i in range(22)])
# Green but short of the matrix means the rest have not been created yet, not that the commit passed.
case("green below min_checks", 3, "WAIT", runs=green(9))
# One page is fetched, so a bigger total means the missing checks could be anything.
case("truncated response", 2, "RESULT: TRUNCATED", runs=green(22), total_count=40)

case("full green matrix", 0, "RESULT: ALL GREEN", runs=green(22))
case("skipped and neutral are green", 0, "RESULT: ALL GREEN",
     runs=green(20) + [check("skip", "completed", "skipped"), check("neut", "completed", "neutral")])
# The whole point of the fail-fast rewrite, a failure is a verdict while the matrix is still running.
case("failure with others running", 1, "RESULT: FAILING -> lint (18 still running)",
     runs=[check("lint", "completed", "failure")] + [check(f"job{i}", "in_progress", None) for i in range(18)],
     min_checks=15)
case("failure below min_checks", 1, "RESULT: FAILING -> lint",
     runs=[check("lint", "completed", "failure")])
# CI auto-cancels a superseded run, and cancelled is the absence of a verdict rather than a failure.
case("all cancelled", 2, "RESULT: CANCELLED", runs=[check(f"job{i}", "completed", "cancelled") for i in range(22)])
# A cancellation must never bury a real failure in the same run.
case("failure outranks cancelled", 1, "RESULT: FAILING -> lint",
     runs=green(20) + [check("lint", "completed", "failure"), check("c", "completed", "cancelled")])
case("timed_out check is a failure", 1, "RESULT: FAILING -> hang",
     runs=green(21) + [check("hang", "completed", "timed_out")])

# A whole matrix leg going red used to print every name on the RESULT line, burying the verdict.
case("many failures stay readable", 1, "RESULT: FAILING -> f0, f1, f2 and 9 more",
     runs=[check(f"f{i}", "completed", "failure") for i in range(12)] + green(10))

case("named green while the rest run", 0, "RESULT: ALL GREEN", names=["a"],
     runs=[check("a", "completed", "success")] + [check(f"job{i}", "in_progress", None) for i in range(21)])
case("named still running", 3, "WAIT", names=["a"], runs=[check("a", "in_progress", None)])
case("named failure", 1, "RESULT: FAILING -> a", names=["a"], runs=[check("a", "completed", "failure")] + green(21))
# A named check absent from a finished run is a typo, and waiting out the horizon would never find it.
case("named typo once the run is over", 2, "RESULT: NOT_FOUND -> typo - never ran, check the names",
     names=["typo"], runs=green(22))
# A typo must not swallow the results of the names that were spelled right.
case("named typo still reports its siblings", 2, "RESULT: NOT_FOUND -> typo", contains="job0: success",
     names=["job0", "typo"], runs=green(22))
# The same absence mid-run only means the job has not started yet.
case("named absent while the run goes on", 3, "WAIT", names=["late"],
     runs=green(10) + [check(f"job{i}", "queued", None) for i in range(10, 22)])

failed = 0
for label, expected_code, expected_prefix, contains, kwargs in CASES:
    code, out = run(**kwargs)
    last = out.splitlines()[-1]
    if code == expected_code and last.startswith(expected_prefix) and (contains is None or contains in out):
        print(f"ok   {label}")
        continue
    failed += 1
    print(f"FAIL {label}: got exit {code} {last!r}, want exit {expected_code} starting {expected_prefix!r}")

print(f"\n{len(CASES) - failed}/{len(CASES)} passed")
sys.exit(1 if failed else 0)
