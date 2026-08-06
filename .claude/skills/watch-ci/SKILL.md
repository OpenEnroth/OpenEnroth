---
name: watch-ci
description: Wait for GitHub CI on a specific commit and report a trustworthy verdict - green, failing, cancelled, or timed out. Use whenever a push needs a CI verdict, instead of hand-writing a polling loop.
---

Run `watch-ci.sh` from this directory as a background task, then read its output file when the task
completes. Do not hand-write a new polling loop - every hand-written one in the history of this project
shipped a false-verdict bug, and this script exists to guard against the three that actually happened:

1. **False pass on an empty set.** A transient empty API response made `pending == 0` vacuously true, and
   the watcher reported "FAILING: none" while every check was still queued. The script refuses to render a
   verdict until at least `MIN_CHECKS` check runs exist.
2. **False fail on a not-started set.** A watcher timed out while all checks were still queued, and its
   final print classified `conclusion: null` as a failure. The script reports `TIMED_OUT` as its own
   outcome, with per-state counts, and says explicitly that it is not a pass/fail verdict.
3. **Stale target.** A watcher kept watching a SHA that a force-push had already superseded. Pass the SHA
   explicitly - never let the script resolve `HEAD` on its own - and when you push a new commit, stop the
   old watcher task and start a new one for the new SHA.
4. **Cancelled read as failed.** CI auto-cancels the run that a new push supersedes, and a watcher still
   pointed at the old SHA sees `conclusion: cancelled` on every check. That is not a failure, it is the
   absence of a verdict, so the script reports `CANCELLED` separately.

## Usage

```sh
.claude/skills/watch-ci/watch-ci.sh <sha> [minutes] [min_checks] [repo]
```

The script calls `gh api`, so it needs either a logged-in `gh` or a `GH_TOKEN` in the environment.

- `sha` - the exact commit to watch. Required.
- `minutes` - polling horizon, default 150. Runs from other PRs compete for the account's job concurrency
  cap, so the queue can back up for an hour or more before your jobs even start. Don't shorten this
  without a reason.
- `min_checks` - minimum check runs that must exist before any verdict, default 15. The full matrix on
  this project is 22.
- `repo` - default `OpenEnroth/OpenEnroth`.

Run it with `run_in_background`, and pipe nothing into it. Exit code 0 means all green, 1 means failing,
and 2 means there is no verdict to report. The last line is always one of `RESULT: ALL GREEN`,
`RESULT: FAILING -> <names>`, `RESULT: CANCELLED -> <names>`, or `RESULT: TIMED_OUT`.

## When it reports FAILING

Get the failing job's log through the check-run id, filtering any tokens out of everything you print:

```sh
gh api "repos/<repo>/commits/<sha>/check-runs?per_page=100" \
    --jq '.check_runs[] | select(.conclusion != "success") | {id, name}'
gh api "repos/<repo>/actions/jobs/<id>/logs" | grep -E "FAILED|error:" | head -30
```

A failure on one platform only is usually real - this project's tests run on four Windows configs, Linux,
macOS and Android, and platform-dependent behavior (libc error text, directory `fopen` semantics, 16-bit
`wchar_t`) has broken exactly one platform before while everything else stayed green.

## When it reports CANCELLED

Almost always the SHA is stale - a newer push cancelled its run. Check what the branch points at now, and
watch that SHA instead. Never report a cancelled run as a CI failure.

## Hygiene

The token comes in through `GH_TOKEN` and must never appear in output. If you print any git or API output
that could embed a token, pipe it through `sed -E 's/github_pat_[A-Za-z0-9_]+/***/g'` first.
