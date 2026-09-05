---
name: watch-ci
description: Wait for GitHub CI on a specific commit and report a trustworthy verdict - green, failing, cancelled, or timed out. Use whenever a push needs a CI verdict, instead of hand-writing a polling loop.
---

Run `watch-ci.py` from this directory as a background task, then read its output file when the task
completes. Do not hand-write a new polling loop - every hand-written one in the history of this project
shipped a false-verdict bug, and this script exists to guard against the four that actually happened:

1. **False pass on an empty set.** A transient empty API response made `pending == 0` vacuously true, and
   the watcher reported "FAILING: none" while every check was still queued. The script refuses to render a
   green verdict until at least `-c` check runs exist. A failure still reports at once, one failed check
   settles the question on its own.
2. **False fail on a not-started set.** A watcher timed out while all checks were still queued, and its
   final print classified `conclusion: null` as a failure. The script reports `TIMED_OUT` as its own
   outcome, carrying the counts it last saw, and says explicitly that it is not a pass/fail verdict.
3. **Stale target.** A watcher kept watching a SHA that a force-push had already superseded. Pass the SHA
   explicitly - never let the script resolve `HEAD` on its own - and when you push a new commit, stop the
   old watcher task and start a new one for the new SHA.
4. **Cancelled read as failed.** CI auto-cancels the run that a new push supersedes, and a watcher still
   pointed at the old SHA sees `conclusion: cancelled` on every check. That is not a failure, it is the
   absence of a verdict, so the script reports `CANCELLED` separately.

## Usage

```sh
.claude/skills/watch-ci/watch-ci.py <sha> [-m minutes] [-c min_checks] [-r repo] [check-name ...]
```

The script calls `gh api`, so it needs either a logged-in `gh` or a `GH_TOKEN` in the environment.

- `sha` - the exact commit to watch, a hex sha. Required. Anything else is rejected outright.
- `-m minutes` - polling horizon, default 150. Runs from other PRs compete for the account's job concurrency
  cap, so the queue can back up for an hour or more before your jobs even start. Don't shorten this
  without a reason.
- `-c min_checks` - minimum check runs that must exist before an ALL GREEN verdict, default 15. The full
  matrix on this project is 22. A failure needs no minimum, one failed check is a verdict on its own. With
  check names given it does not gate the green at all, the named checks alone decide that.
- `-r repo` - default `OpenEnroth/OpenEnroth`.
- `check-name ...` - optional. With names given, the script waits until every named check has completed and
  reports each one's conclusion, whatever the rest of the run is doing. Use it when you need the results of
  specific jobs - both darwin legs, say - rather than the first verdict.

Without names the script returns on the first failure, or once everything has completed green. It polls
once a minute and prints a line whenever the completed count changes, so a background task's output shows
progress.

The verdict itself is `verdict.py`, a module that turns one API response into one outcome with no network
and no clock. That keeps it testable - run `verdict_test.py` after touching it, it replays canned responses
for every outcome except TIMED_OUT, which belongs to the polling loop rather than the verdict.

Run it with `run_in_background`. Exit code 0 means all green, 1 means failing,
and 2 means there is no verdict to report. The last line is always one of `RESULT: ALL GREEN`,
`RESULT: FAILING -> <names>`, `RESULT: CANCELLED -> <names>`, `RESULT: NOT_FOUND -> <names>`,
`RESULT: TRUNCATED`, `RESULT: NO_ACCESS`, `RESULT: NO_GH`, `RESULT: CRASHED`, or `RESULT: TIMED_OUT`.

A FAILING line lists every failed job with its URL on the lines above it, and caps the names it repeats when a
whole matrix leg goes red. Without names it also says how many checks were still running - those got no
verdict. NOT_FOUND means what you asked for does not exist, either a mistyped sha or repo, or a check name
that never ran once the rest of the run had finished. TRUNCATED means the pages read did not add up to the
total the API reported, so some check runs were never seen and no green verdict is possible. NO_ACCESS, NO_GH
and CRASHED are the script's own failures rather than CI's, and none of them is a pass or a fail.

A commit keeps every check run that ever ran on it, so a re-run leaves the run it replaced sitting alongside
it, and the two often disagree. Only the newest run of each name counts. Ignoring that produced both a green
verdict on a check whose latest run had failed and a red one from a run superseded months earlier, on real
commits in this repository.

## When it reports FAILING

The job URL is printed with the verdict - its trailing number is the job id. Get the log through it,
filtering any tokens out of everything you print:

```sh
gh api "repos/<repo>/actions/jobs/<id>/logs" | grep -E "FAILED|error:" | head -30
```

The run may still be going for the other checks. Fix what failed rather than waiting for the rest, a new push
cancels them anyway.

A failure on one platform only is usually real - this project's tests run on four Windows configs, Linux,
macOS and Android, and platform-dependent behavior (libc error text, directory `fopen` semantics, 16-bit
`wchar_t`) has broken exactly one platform before while everything else stayed green.

## When it reports CANCELLED

Almost always the SHA is stale - a newer push cancelled its run. Check what the branch points at now, and
watch that SHA instead. Never report a cancelled run as a CI failure. A `stale` check run is reported the
same way, for the same reason.

Read the `RESULT:` line, not the exit status of the surrounding job. A watcher that is killed, or piped
through anything, reports its own exit code, and a task that ends without a `RESULT:` line has told you
nothing about CI.

## Hygiene

The token comes in through `GH_TOKEN` and must never appear in output. If you print any git or API output
that could embed a token, pipe it through `sed -E 's/github_pat_[A-Za-z0-9_]+/***/g'` first.
