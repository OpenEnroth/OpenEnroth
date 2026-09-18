---
name: summary
description: Tabulate one session's work as two short tables, its pull requests and issues with live status, and what is still pending with whose move it is. Use for a summary, a recap, session status, "where are we", "what's left", "catch me up", the state of this session's PRs, issues or branches, or a fresh agent asked where things stand after a handoff.
---

Answer with two short tables and almost no prose. The first is this session's pull requests and
issues. The second is what is still pending and whose move it is.

## Rules

**This session only.** Many agents run here at once, one worktree each, all under one GitHub
login. Another agent's pull request, issue or stranded commit must never appear. `gather.py`
scopes itself from this session's transcript. If it prints `NO SESSION SCOPE`, say that the scope
could not be established and stop, rather than widening it.

**Live, never recalled.** Every status and every description comes from `gather.py` output made
now. A row the output lacks is gathered again with `--pr N`, never written from memory.

**Read-only.** A summary is an answer. Do not commit, push, delete, tidy or start on the pending
items while producing it.

## Gather

```sh
<skill-dir>/gather.py
```

`<skill-dir>` is the directory holding this file, which the harness prints when the skill loads.
The run takes about five seconds and prints three things.

- The worktrees this session entered, committed in or edited, each with its uncommitted files,
  its commits on no remote-tracking ref, and its branch's pull request.
- Each pull request or issue this session created or wrote to, with live state, checks, review
  decision, unresolved review threads, labels and an `about:` line from its own description.
- The numbers the session only read, which are left out on purpose.

`--pr N` adds a pull request or an issue the transcript missed, such as one the user opened by
hand or one pushed to under a different branch name. `--session ID` reads another session.
`--all-worktrees` shows the whole checkout, and is only for when the user asks for that.

The transcript on disk survives compaction, so this works late in a long session. A fresh agent
working from a handoff has an empty transcript. It reads the handoff, passes every number the
handoff names with `--pr`, and trusts the live output wherever the two disagree.

## First table: pull requests and issues

| Item | What it is | Status |

Rows are the pull requests and issues in the gather output, newest first, and nothing else.
Anything without a number on GitHub stays out, which covers an investigation, a review verdict,
a finding and a branch. If such a thing needs action it is a row in the second table. If it needs
none it gets at most one line of prose under the table. Drop a gathered row only when the session
did no work on it.

- **Item** is the linked number.
- **What it is** says what the item is about, not what this session did to it. The reader has
  probably never opened it, so lead with what it fixes or changes, taken from the title and the
  `about:` line. "Review fixes pushed, exact end-time assertion, extra ticks after the wait"
  describes an afternoon and leaves the reader with no idea what the pull request is for. When
  the session did not write the whole thing, add its part as a second sentence, as in "Ensrick's
  test that waiting until dawn ends at 5:00. This session pushed review fixes."
- **Status** is the gathered state, trimmed. `Merged 2026-09-10`, `Open, all 21 checks green,
  approved`, `Open, 14/21 green, 7 still running`, `Open, CONFLICTING`, `Filed 2026-09-17, open`.

With no pull requests and no issues, say so in one line and print no table.

## Second table: pending

| Item | What it is | Pending | Whose move |

A row is something a person has to decide, or real work that is still outstanding. **Pending** is
one of these.

- `Decision` means the user has to choose.
- `File issue` is a finding worth an issue that nobody has filed.
- `Open PR` is a change that still needs its own pull request, whether or not the code exists
  yet. Uncommitted work, and commits on a branch with no pull request, are both this.
- `Review reply` is unresolved review threads.
- `Fix CI` is failing checks.
- `Rebase` is a conflicting pull request.
- `Verify` is a claim the session made and nobody checked.
- `Save tool` is a script the next session would otherwise write again from nothing.

**Whose move** is `You` or `Me`. It is what lets the user answer the table line by line, so the
user's rows come first.

Three things stay out.

- An open pull request that is only waiting for review. Its status in the first table says so.
- Housekeeping. Worktrees to delete, stashes, build trees, patch files, logs and scratch output
  are never rows and are not worth a mention.
- The steps of one item. It is one row per decision or deliverable.

More than five rows means one of those crept in. With nothing pending, say so in one line.

## Where to run it

In the main session. A subagent's report is not shown to the user, and a fork pays for a full copy
of the context before it starts, so neither saves anything. Do not publish an artifact unless
asked.
