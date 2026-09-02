"""Turns one check-runs API response into a CI verdict, with no network and no clock of its own.

0 is all green, 1 is failing and 2 is no verdict to report, which are the codes watch-ci.py exits with. WAIT
is the one it keeps to itself, and means the response settled nothing."""
WAIT = 3
BAD = ("failure", "timed_out", "action_required", "startup_failure")
NO_VERDICT = ("cancelled", "stale")


def summarize(names):
    """Joins check names for the RESULT line, keeping it readable when a whole matrix leg went red.

    @param names                        Failing check names, already listed in full above with their URLs.
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


def decide_named(names, runs, run_over):
    """Reports the named checks once they have all completed, ignoring whatever the rest of the run is doing.

    @param names                        Check names asked for on the command line.
    @param runs                         Check runs returned for the commit.
    @param run_over                     Whether every check on the commit has completed.
    @return                             Exit code and lines, WAIT while any named check is still outstanding.
    """
    present = {r["name"]: r for r in runs}
    missing = [n for n in names if n not in present]
    named = [present[n] for n in names if n in present]
    pending = [r for r in named if r["status"] != "completed"]

    # A name still absent once the run is over is a typo, and waiting out the horizon would not find it.
    if missing and run_over:
        lines = [describe(r) for r in named]
        never_ran = " (never ran: " + summarize(missing) + ")"
        # A failure among the names that did run outranks the ones that did not, the same way it outranks a
        # cancellation below. Reporting only the typo would bury it.
        bad = [r["name"] for r in named if r["conclusion"] in BAD]
        if bad:
            return 1, lines + ["RESULT: FAILING -> " + summarize(bad) + never_ran]
        return 2, lines + ["RESULT: NOT_FOUND -> " + summarize(missing) + " - never ran, check the names"]
    if missing or pending:
        detail = f"named {len(named) - len(pending)}/{len(names)} completed"
        if missing:
            detail += ", not started: " + ", ".join(missing)
        return WAIT, ["WAIT " + detail]

    lines = [describe(r) for r in named]
    bad = [r["name"] for r in named if r["conclusion"] in BAD]
    if bad:
        return 1, lines + ["RESULT: FAILING -> " + summarize(bad)]
    cancelled = [r["name"] for r in named if r["conclusion"] in NO_VERDICT]
    if cancelled:
        return 2, lines + ["RESULT: CANCELLED -> " + summarize(cancelled) + " - not a pass/fail verdict"]
    return 0, lines + ["RESULT: ALL GREEN"]


def decide(payload, min_checks, names):
    """Decides the outcome for one check-runs response.

    @param payload                      Parsed response, or None when the body did not parse.
    @param min_checks                   Minimum check runs that must exist before a green verdict.
    @param names                        Check names asked for, empty to judge the whole run.
    @return                             Exit code and the lines to print, the last of which is the RESULT or WAIT line.
    """
    if not isinstance(payload, dict):
        return WAIT, ["WAIT api error"]
    runs = payload.get("check_runs")
    if runs is None:
        message = payload.get("message", "")
        # A sha or repo that does not exist never turns into check runs, however long we wait for it.
        if str(payload.get("status", "")) in ("404", "422"):
            return 2, [f"RESULT: NOT_FOUND -> {message} - not a pass/fail verdict"]
        return WAIT, ["WAIT api error" + (f": {message}" if message else "")]

    # One page is fetched, so a larger total means checks are missing here and no count below can be trusted.
    truncated = payload.get("total_count", len(runs)) > len(runs)
    completed = [r for r in runs if r["status"] == "completed"]
    # A full set that has all finished means no further check run will appear for this commit.
    run_over = not truncated and len(runs) >= min_checks and len(completed) == len(runs)

    if names:
        return decide_named(names, runs, run_over)

    # A failure is final the moment it lands, whatever the rest is doing. Cancelled checks carry no verdict, but
    # a failure elsewhere in the same run is still a failure, and reporting CANCELLED would bury it.
    bad = [r for r in completed if r["conclusion"] in BAD]
    if bad:
        still = len(runs) - len(completed)
        rest = f" ({still} still running)" if still else ""
        lines = [describe(r) for r in bad]
        return 1, lines + ["RESULT: FAILING -> " + summarize([r["name"] for r in bad]) + rest]

    if truncated:
        return 2, [f"RESULT: TRUNCATED - {payload['total_count']} checks exist, {len(runs)} returned"
                   " - not a pass/fail verdict"]
    if len(runs) < min_checks or len(completed) < len(runs):
        return WAIT, [f"WAIT {len(completed)}/{len(runs)} completed, min {min_checks}"]

    cancelled = [r["name"] for r in runs if r["conclusion"] in NO_VERDICT]
    if cancelled:
        return 2, ["RESULT: CANCELLED -> " + summarize(cancelled) + " - not a pass/fail verdict"]
    return 0, ["RESULT: ALL GREEN"]
