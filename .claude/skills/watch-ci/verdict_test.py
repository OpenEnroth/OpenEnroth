#!/usr/bin/env python3
"""Checks the verdict against canned API responses. Run it after touching verdict.py."""
import json
import os
import subprocess
import sys

# Importing the verdict keeps the checks below a plain function call, and leaves no __pycache__ in the skill.
sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import verdict

URL = "https://github.com/o/r/actions/runs/1/job/2"


def check(name, status, conclusion):
    return {"name": name, "status": status, "conclusion": conclusion, "html_url": URL}


def green(count, first=0):
    return [check(f"job{i}", "completed", "success") for i in range(first, first + count)]


def decide(runs=(), names=(), min_checks=15, total_count=None):
    """Calls the verdict on a response assembled from the given check runs.

    @param runs                         Check runs to put in the response.
    @param names                        Check names to judge, empty for the whole run.
    @param min_checks                   Minimum check runs before a green verdict.
    @param total_count                  Reported total, defaults to the number of runs.
    @return                             Exit code and the lines printed.
    """
    total = len(runs) if total_count is None else total_count
    return verdict.decide({"total_count": total, "check_runs": list(runs)}, min_checks, list(names))


CASES = []


def case(label, code, prefix, contains=None, **kwargs):
    CASES.append((label, code, prefix, contains, kwargs))


# A transient empty response once made "no pending checks" vacuously true and produced "FAILING: none".
case("empty response", verdict.WAIT, "WAIT", runs=[])
# A watcher once timed out with every check still queued and read conclusion null as a failure.
case("all queued", verdict.WAIT, "WAIT", runs=[check(f"job{i}", "queued", None) for i in range(22)])
# Green but short of the matrix means the rest have not been created yet, not that the commit passed.
case("green below min_checks", verdict.WAIT, "WAIT", runs=green(9))
# One page is fetched, so a bigger total means the missing checks could be anything.
case("truncated response", 2, "RESULT: TRUNCATED", runs=green(22), total_count=40)

case("full green matrix", 0, "RESULT: ALL GREEN", runs=green(22))
case("skipped and neutral are green", 0, "RESULT: ALL GREEN",
     runs=green(20) + [check("skip", "completed", "skipped"), check("neut", "completed", "neutral")])
# The whole point of the fail-fast rewrite, a failure is a verdict while the matrix is still running.
case("failure with others running", 1, "RESULT: FAILING -> lint (18 still running)",
     runs=[check("lint", "completed", "failure")] + [check(f"job{i}", "in_progress", None) for i in range(18)])
case("failure below min_checks", 1, "RESULT: FAILING -> lint", runs=[check("lint", "completed", "failure")])
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
case("named still running", verdict.WAIT, "WAIT", names=["a"], runs=[check("a", "in_progress", None)])
case("named failure", 1, "RESULT: FAILING -> a", names=["a"], runs=[check("a", "completed", "failure")] + green(21))
# A named check absent from a finished run is a typo, and waiting out the horizon would never find it.
case("named typo once the run is over", 2, "RESULT: NOT_FOUND -> typo - never ran, check the names",
     names=["typo"], runs=green(22))
# A typo must not swallow the results of the names that were spelled right.
case("named typo still reports its siblings", 2, "RESULT: NOT_FOUND -> typo", contains="job0: success",
     names=["job0", "typo"], runs=green(22))
# The same absence mid-run only means the job has not started yet.
case("named absent while the run goes on", verdict.WAIT, "WAIT", names=["late"],
     runs=green(10) + [check(f"job{i}", "queued", None) for i in range(10, 22)])

failed = 0
for label, want_code, prefix, contains, kwargs in CASES:
    code, lines = decide(**kwargs)
    if code == want_code and lines[-1].startswith(prefix) and (contains is None or contains in lines):
        print(f"ok   {label}")
        continue
    failed += 1
    print(f"FAIL {label}: got exit {code} {lines[-1]!r}, want exit {want_code} starting {prefix!r}")


def shim(body, args, want_code, want_last):
    """Runs verdict.py as watch-ci.sh runs it, to pin the stdin and exit code wiring the shell depends on.

    @param body                         Raw stdin for the script.
    @param args                         Command line arguments after the script name.
    @param want_code                    Expected exit code.
    @param want_last                    Expected prefix of the last printed line.
    @return                             Whether the script behaved as expected.
    """
    script = os.path.join(os.path.dirname(os.path.abspath(__file__)), "verdict.py")
    done = subprocess.run([sys.executable, script] + args, input=body, capture_output=True, text=True)
    last = done.stdout.strip().splitlines()[-1]
    return done.returncode == want_code and last.startswith(want_last)


# The shell reads the exit code, so a body that is not JSON at all must still poll rather than pass or fail.
SHIMS = [
    ("malformed body polls again", "<html>502</html>", ["15"], verdict.WAIT, "WAIT api error"),
    ("green matrix exits 0", json.dumps({"total_count": 22, "check_runs": green(22)}), ["15"], 0, "RESULT: ALL GREEN"),
    ("names arrive as arguments", json.dumps({"total_count": 22, "check_runs": green(22)}), ["15", "job3"],
     0, "RESULT: ALL GREEN"),
]
for label, body, args, want_code, want_last in SHIMS:
    if shim(body, args, want_code, want_last):
        print(f"ok   {label}")
        continue
    failed += 1
    print(f"FAIL {label}")

total = len(CASES) + len(SHIMS)
print(f"\n{total - failed}/{total} passed")
sys.exit(1 if failed else 0)
