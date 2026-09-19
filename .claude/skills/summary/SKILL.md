---
name: summary
description: Tabulate one session's work as two short tables, its pull requests and issues with live status, and what is still pending with whose move it is. Use for a summary, a recap, session status, "where are we", "what's left", "catch me up", the state of this session's PRs, issues or branches, or a fresh agent asked where things stand after a handoff.
---

Answer with two short tables and almost no prose. The first is this session's pull requests and
issues. The second is what is still pending and whose move it is.

## Rules

**This session only.** Many agents run here at once, one worktree each, all under one GitHub
login. Report only what this conversation did. Never sweep every worktree, every open pull request
or every transcript, because that reports other agents' work as this session's.

**Live, never recalled.** Which items belong to the session comes from the conversation. Their
state, and what they are about, come from `status.py` run now.

**Read-only.** A summary is an answer. Do not commit, push, delete, tidy or start on the pending
items while producing it.

## Gather

1. List the pull requests and issues this session created or changed, meaning opened, pushed to,
   commented on, edited, closed or merged. One it only read is not on the list. If the
   conversation was compacted and the list may be incomplete, print every number this session's
   transcripts link to, and keep the ones the session worked on.

   ```sh
   <skill-dir>/status.py --candidates
   ```

2. Get the live state of the list. `<skill-dir>` is the directory holding this file.

   ```sh
   <skill-dir>/status.py 2768 2763
   ```

   Each number gets a block with its kind, state, checks, review decision, unresolved review
   threads, labels, an `about:` line from its own description, and a `cell:` line.

3. In each worktree the session worked in, look for work that never left it.

   ```sh
   git status --short
   git log --oneline HEAD --not --remotes
   ```

A fresh agent working from a handoff has only the handoff. It passes every number the handoff
names to `status.py`, and trusts the live output wherever the two disagree.

## First table: pull requests and issues

| Item | What it is | Status |

Rows are the pull requests and issues from step 1, newest first, and nothing else. Anything
without a number on GitHub stays out, which covers an investigation, a review verdict, a finding
and a branch. If such a thing needs action it is a row in the second table. If it needs none it
gets at most one line of prose under the table.

- **Item** is the `cell:` line, copied as it stands. It reads `[PR #2768](url)` for a pull
  request and `[Issue #2763](url)` for an issue. Never a bare `#2768`.
- **What it is** says what the item is about, not what this session did to it. The reader has
  probably never opened it, so lead with what it fixes or changes, taken from the title and the
  `about:` line. "Review fixes pushed, exact end-time assertion, extra ticks after the wait"
  describes an afternoon and leaves the reader with no idea what the pull request is for. When
  the session did not write the whole thing, add its part as a second sentence, as in "Ensrick's
  test that waiting until dawn ends at 5:00. This session pushed review fixes."
- **Status** is the `status:` line, trimmed. `Merged 2026-09-10`, `Open, 21/21 checks green,
  approved`, `Open, 14/21 checks green, 7 pending or cancelled`, `Open, filed 2026-09-17`.

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

**Item** takes the `PR #2768` or `Issue #2763` form when the row is about one, and a short name
otherwise.

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
