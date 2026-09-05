#!/usr/bin/env python3
"""Checks the verdict against canned API responses. Run it after touching verdict.py."""
import sys

sys.dont_write_bytecode = True  # Set before the import, so the skill directory never grows a __pycache__.
import verdict

URL = "https://github.com/o/r/actions/runs/1/job/2"


def check(name, status, conclusion, started="2026-01-01T00:00:00Z", ident=1):
    return {"name": name, "status": status, "conclusion": conclusion, "html_url": URL,
            "started_at": started, "id": ident}


def green(count):
    return [check(f"job{i}", "completed", "success") for i in range(count)]


def decide(runs=(), names=(), min_checks=15, total_count=None, payload=None):
    """Calls the verdict on a response assembled from the given check runs.

    @param runs                         Check runs to put in the response.
    @param names                        Check names to judge, empty for the whole run.
    @param min_checks                   Minimum check runs before a green verdict.
    @param total_count                  Reported total, defaults to the number of runs.
    @param payload                      Whole response, overriding the fields above.
    @return                             Exit code and the lines printed.
    """
    if payload is None:
        total = len(runs) if total_count is None else total_count
        payload = {"check_runs": list(runs)}
        if total is not MISSING:
            payload["total_count"] = total
    return verdict.decide(None if payload is NOT_JSON else payload, min_checks, list(names))


NOT_JSON = object()  # What fetch() hands over when the response was not JSON at all.
MISSING = object()   # Stands for a total_count that the response did not carry.

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

# A mistyped sha returns a 422 error body on stdout, and waiting out the horizon would never resolve it.
case("bad sha is not a wait", 2, "RESULT: NOT_FOUND -> No commit found for SHA: dead",
     payload={"message": "No commit found for SHA: dead", "status": "422"})
case("missing repo is not a wait", 2, "RESULT: NOT_FOUND", payload={"message": "Not Found", "status": "404"})
# Bad or absent credentials never come right on their own, so this one used to poll out the horizon too.
case("no credentials is not a wait", 2, "RESULT: NO_ACCESS",
     payload={"message": "gh auth login", "status": "401"})
# A server error is transient, so that one does keep polling.
case("server error keeps waiting", verdict.WAIT, "WAIT api error", payload={"message": "Server Error"})

case("unparseable body polls again", verdict.WAIT, "WAIT api error", payload=NOT_JSON)

# The one case that must never be reached by accident, so it is pinned against the whole matrix.
case("full green matrix", 0, "RESULT: ALL GREEN", runs=green(22))
# Nothing else records that a path-filtered or neutral check is not a failure, it is only an absence from BAD.
case("skipped and neutral are green", 0, "RESULT: ALL GREEN",
     runs=green(20) + [check("skip", "completed", "skipped"), check("neut", "completed", "neutral")])
# The whole point of the fail-fast rewrite, a failure is a verdict while the matrix is still running.
case("failure with others running", 1, "RESULT: FAILING -> lint (18 still running)",
     runs=[check("lint", "completed", "failure")] + [check(f"job{i}", "in_progress", None) for i in range(18)])
# min_checks gates green and never gates a failure, an asymmetry that reads like an oversight and is not one.
case("failure below min_checks", 1, "RESULT: FAILING -> lint", runs=[check("lint", "completed", "failure")])
# CI auto-cancels a superseded run, and cancelled is the absence of a verdict rather than a failure.
case("all cancelled", 2, "RESULT: CANCELLED", runs=[check(f"job{i}", "completed", "cancelled") for i in range(22)])
# A cancellation must never bury a real failure in the same run.
case("failure outranks cancelled", 1, "RESULT: FAILING -> lint",
     runs=green(20) + [check("lint", "completed", "failure"), check("c", "completed", "cancelled")])
# A check whose conclusion is timed_out is a failure, unlike this script's own TIMED_OUT, which is not.
case("timed_out check is a failure", 1, "RESULT: FAILING -> hang",
     runs=green(21) + [check("hang", "completed", "timed_out")])
# A whole matrix leg going red used to print every name on the RESULT line, burying the verdict.
case("many failures stay readable", 1, "RESULT: FAILING -> f0, f1, f2 and 9 more",
     runs=[check(f"f{i}", "completed", "failure") for i in range(12)] + green(10))

# Naming a check means the rest of the matrix does not get a say, which is the whole point of naming one.
# A re-run leaves both runs on the commit, and the superseded one used to win and report a stale verdict.
# Proven on OpenEnroth c91e7768, where the newest build_data_cache failed while an older one had passed.
NEW = "2026-02-02T00:00:00Z"
OLD = "2026-01-01T00:00:00Z"
case("a re-run that failed beats the pass it replaced", 1, "RESULT: FAILING -> flaky",
     runs=green(21) + [check("flaky", "completed", "success", OLD, 1),
                       check("flaky", "completed", "failure", NEW, 2)])
# The mirror image, proven on OpenEnroth 297fe4ba, where a 2026-05-06 failure was reported for months.
case("a superseded failure does not fail the commit", 0, "RESULT: ALL GREEN",
     runs=green(21) + [check("flaky", "completed", "failure", OLD, 1),
                       check("flaky", "completed", "success", NEW, 2)])
case("a re-run still going is not a verdict", verdict.WAIT, "WAIT",
     runs=green(21) + [check("flaky", "completed", "success", OLD, 1),
                       check("flaky", "in_progress", None, NEW, 2)])
# Only three conclusions count as green, so a new one GitHub invents cannot slip through as a pass.
case("an unknown conclusion is not green", 1, "RESULT: FAILING -> weird",
     runs=green(21) + [check("weird", "completed", "exploded")])
case("a completed check with no conclusion is not green", 1, "RESULT: FAILING -> null",
     runs=green(21) + [check("null", "completed", None)])
# Without total_count there is no way to know a page was the whole of it.
case("a response with no total_count is truncated", 2, "RESULT: TRUNCATED", runs=green(22), total_count=MISSING)
# min_checks gates green, and gating never-ran on it left a typo waiting out the horizon on a small matrix.
case("a typo on a short run still reports", 2, "RESULT: NOT_FOUND -> typo", names=["typo"], runs=green(3))
# A named check found on page one may have a newer run on a page that was never read.
case("named checks are not judged off a partial response", 2, "RESULT: TRUNCATED", names=["job0"],
     runs=green(22), total_count=40)

case("named green while the rest run", 0, "RESULT: ALL GREEN", names=["a"],
     runs=[check("a", "completed", "success")] + [check(f"job{i}", "in_progress", None) for i in range(21)])
case("named still running", verdict.WAIT, "WAIT", names=["a"], runs=[check("a", "in_progress", None)])
case("named failure", 1, "RESULT: FAILING -> a", names=["a"], runs=[check("a", "completed", "failure")] + green(21))
# A name that never ran used to outrank a check that really failed, and reported NOT_FOUND over the failure.
case("typo does not bury a real failure", 1, "RESULT: FAILING -> a (never ran: typo)", names=["a", "typo"],
     runs=[check("a", "completed", "failure")] + green(21))
# A superseded run cancels every check at once, so this is the list most likely to bury the verdict.
case("whole matrix cancelled stays readable", 2, "RESULT: CANCELLED -> job0, job1, job2 and 19 more",
     runs=[check(f"job{i}", "completed", "cancelled") for i in range(22)])
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
    want = f"exit {want_code} starting {prefix!r}" + (f" with {contains!r} among the lines" if contains else "")
    print(f"FAIL {label}: got exit {code} {lines[-1]!r}, want {want}")


print(f"\n{len(CASES) - failed}/{len(CASES)} passed")
sys.exit(1 if failed else 0)
