"""Turns one check-runs API response into a CI verdict, with no network and no clock of its own.

0 is all green, 1 is failing and 2 is no verdict to report, which are the codes watch-ci.py exits with. WAIT
is the one it keeps to itself, and means the response settled nothing."""
WAIT = 3
GREEN = ("success", "neutral", "skipped")
NO_VERDICT = ("cancelled", "stale")


def latest(runs):
    """Keeps only the newest run of each check, so a re-run decides the check and the runs it replaced do not.

    @param runs                         All check runs the API returned for the commit.
    @return                             One run per name, the most recently started of each.
    """
    newest = {}
    for run in runs:
        when = (run.get("started_at") or "", run.get("id") or 0)
        if when >= newest.get(run["name"], ((), ()))[0]:
            newest[run["name"]] = (when, run)
    return [run for _, run in newest.values()]


def failed(run):
    """Says whether a completed check run is a failure, treating anything unrecognized as one.

    @param run                          Check run object from the API.
    @return                             True when the run did not finish in a state that counts as green.
    """
    return run["conclusion"] not in GREEN and run["conclusion"] not in NO_VERDICT


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
    where = " " + (run.get("html_url") or "") if failed(run) else ""
    return f'{run["name"]}: {run["conclusion"] or run["status"]}{where}'


def decide_named(names, runs, run_over, truncated):
    """Reports the named checks once they have all completed, ignoring whatever the rest of the run is doing.

    @param names                        Check names asked for on the command line.
    @param runs                         Check runs returned for the commit.
    @param run_over                     Whether every check on the commit has completed.
    @return                             Exit code and lines, WAIT while any named check is still outstanding.
    """
    present = {r["name"]: r for r in runs}  # One run per name already, latest() saw to that.
    missing = [n for n in names if n not in present]
    named = [present[n] for n in names if n in present]
    pending = [r for r in named if r["status"] != "completed"]

    # A name still absent once the run is over is a typo, and waiting out the horizon would not find it.
    if missing and run_over:
        lines = [describe(r) for r in named]
        never_ran = " (never ran: " + summarize(missing) + ")"
        # A failure among the names that did run outranks the ones that did not, the same way it outranks a
        # cancellation below. Reporting only the typo would bury it.
        bad = [r["name"] for r in named if failed(r)]
        if bad:
            return 1, lines + ["RESULT: FAILING -> " + summarize(bad) + never_ran]
        return 2, lines + ["RESULT: NOT_FOUND -> " + summarize(missing) + " - never ran, check the names"]
    if not missing and not pending and truncated:
        return 2, [f"RESULT: TRUNCATED - a named check may have a newer run that was left out"
                   " - not a pass/fail verdict"]
    if missing or pending:
        detail = f"named {len(named) - len(pending)}/{len(names)} completed"
        if missing:
            detail += ", not started: " + ", ".join(missing)
        return WAIT, ["WAIT " + detail]

    lines = [describe(r) for r in named]
    bad = [r["name"] for r in named if failed(r)]
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
        status = str(payload.get("status", ""))
        # A sha or repo that does not exist never turns into check runs, however long we wait for it.
        if status in ("404", "422"):
            return 2, [f"RESULT: NOT_FOUND -> {message} - not a pass/fail verdict"]
        # Credentials do not repair themselves either, and this one would otherwise poll to the horizon.
        if status == "401":
            return 2, [f"RESULT: NO_ACCESS - {message} - not a pass/fail verdict"]
        return WAIT, ["WAIT api error" + (f": {message}" if message else "")]

    # A response that did not carry every check run cannot support a green verdict or a never-ran one.
    truncated = payload.get("total_count") is None or payload["total_count"] > len(runs)
    runs = latest(runs)
    completed = [r for r in runs if r["status"] == "completed"]
    # Nothing outstanding means no further check run will appear, whatever the size of the matrix.
    run_over = not truncated and bool(runs) and len(completed) == len(runs)

    if names:
        return decide_named(names, runs, run_over, truncated)

    # A failure is final the moment it lands, whatever the rest is doing. Cancelled checks carry no verdict, but
    # a failure elsewhere in the same run is still a failure, and reporting CANCELLED would bury it.
    bad = [r for r in completed if failed(r)]
    if bad:
        still = len(runs) - len(completed)
        rest = f" ({still} still running)" if still else ""
        lines = [describe(r) for r in bad]
        return 1, lines + ["RESULT: FAILING -> " + summarize([r["name"] for r in bad]) + rest]

    if truncated:
        seen = f"{payload['total_count']} checks exist, {len(runs)} returned" if payload.get("total_count") \
            else "the response did not say how many checks exist"
        return 2, [f"RESULT: TRUNCATED - {seen} - not a pass/fail verdict"]
    if len(runs) < min_checks or len(completed) < len(runs):
        return WAIT, [f"WAIT {len(completed)}/{len(runs)} completed, min {min_checks}"]

    cancelled = [r["name"] for r in runs if r["conclusion"] in NO_VERDICT]
    if cancelled:
        return 2, ["RESULT: CANCELLED -> " + summarize(cancelled) + " - not a pass/fail verdict"]
    return 0, ["RESULT: ALL GREEN"]
