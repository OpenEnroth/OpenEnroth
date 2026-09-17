---
name: summary
description: Tabulate one session's work as two tables, what it produced and what is still open, with live pull request, issue and branch status. Use for a summary, a recap, session status, "where are we", "what's left", "catch me up", this session's PRs or branches, or a fresh agent asked where things stand after a handoff.
---

Answer with two tables and almost no prose. The first is the pull requests and issues. The
second is what is still open and who has to move next.

## Two rules

**Scope to this session.** This machine runs many agents in parallel, one worktree per task.
A sweep of every worktree or every transcript reports other agents' pull requests as this
session's work, which is worse than useless because it reads as authoritative. The session
id in `$CLAUDE_CODE_SESSION_ID` is the scope, and the bundled scripts default to it. Never
widen to the whole checkout unless the user asks for that.

**Query live, never recall.** Every status in both tables comes from a query made now.
Pull requests merge, CI turns red and reviewers reply while a session is still running. One
session had to be told "OK some PRs were merged, check upstream, then tell me what's the
current status", which is the failure this rule exists to prevent.

Gather first. Write the tables second.

## Step 1, fix the scope

Default scope is the current session. If the user named something narrower such as one pull
request or one branch, use that. State the scope in one line above the tables so a wrong
guess is visible and cheap to correct.

## Step 2, gather

Run the bundled script with no arguments. It scopes itself to this session and takes about
twelve seconds.

```sh
<skill-dir>/gather.py
```

`<skill-dir>` is the directory holding this SKILL.md, which the harness prints when the skill
loads. It is `.claude/skills/summary` in a repository that checks the skill in, and
`~/.claude/skills/summary` when it is installed for every project.

It reads this session's transcript for the pull requests the session actually touched, then
reports only the worktrees the session worked in, with live pull request state for each.
It says how many other worktrees it hid.

- `--pr N` adds a pull request the transcript missed, repeatable.
- `--issue N` adds an issue the transcript missed, repeatable.
- `--no-issues` skips the search for issues filed while the session ran.
- `--threads` counts unresolved review threads through GraphQL, because REST miscounts them.
- `--all-worktrees` drops the session scope and reports the whole checkout. Only on request.
- `--session ID` summarises a different session.
- `--no-worktrees` skips the worktree sweep when only pull requests matter.

To see what the scoping found before committing to it, or to summarise a session other than
this one, run the transcript reader on its own. It takes about thirty milliseconds.

```sh
<skill-dir>/session_prs.py [--session ID] [--repo owner/name]
```

It labels each number `created`, `operated` or `mentioned`, and reports whether it is a
pull request or an issue.

- `created` means a `gh pr create` or `gh issue create` call returned that number.
- `operated` means the session ran a command naming it.
- `mentioned` is everything else, including links that arrived inside command output.

Trust the first two. A `mentioned` row is usually incidental, so confirm it against the
conversation before it earns a table row. Reading a pull request to write the summary is not
working on it, and neither is a link that scrolled past in a search result.

Issues filed in a browser never appear in a transcript, so `gather.py` also searches for
issues opened by the `gh` user during the session's window and marks those. That backstop is
the reliable path for a filed issue. Transcript matching is text matching, and a command that
merely searches for the words `gh issue create` looks much like one.

If this session built a script worth reusing, save it somewhere durable now and report it as
done. Everything else in a scratch directory is throwaway and never reaches either table.

## Step 3, the first table

Pull requests and issues, newest first, and nothing else. This is the table someone scans to
see what shipped, and a row without a number on GitHub does not answer that question.

An issue the session filed belongs here rather than in the second table, because filing it
was the work. The second table's `Issue` disposition is for one that still needs filing, so a
row graduates from there to here once it exists.

**Everything else stays out.** An investigation, a review verdict, a benchmark or a finding
has no number, so it gets no row. When it settled something and needs nothing further, give
it one line of prose under the table. When it still needs a decision, a commit or a pull
request, it is a row in the second table instead. A branch carrying unshipped commits is that
second case, never a row here.

**Keep it short.** A row is a unit of work someone could review or revert, not a step taken
on the way to one. Research, a capability probe and a bug found and fixed inside your own
unshipped code are all *how* a deliverable got made. A session that produced one pull request
has a one row table, and padding it reads as more progress than there was.

Pull requests the session only queried while building this summary are not work items
either. Drop them.

If the session touched no pull requests and filed no issues, say that in one line instead of
printing a table.

| Item | What it is | Status |

- **Item** is the linked pull request or issue number, and nothing else.
- **What it is** says what the item is about, not what this session did to it. Whoever reads
  the table has probably never opened the pull request, so lead with what it changes or fixes.
  A row reading "review fixes pushed onto the dawn test, exact end-time assertion, extra ticks
  after the wait" describes an afternoon and leaves the reader with no idea what the pull
  request is for. Take the subject from the pull request's own title and body, which
  `gather.py` prints on an `about:` line, and not from what the session remembers doing.
  Where the session did not write the whole thing, name the subject first and add its part
  after, as in "Ensrick's test that rest ends at dawn. This session pushed review fixes."
  One sentence, or two when the second is the session's part.
- **Status** is live and specific. `Merged 2026-09-10`, `Open, all 21 checks green, review
  required`, `Open, 14/21 green, 7 still running`, `Open, CONFLICTING so no checks queued`,
  `Draft`. An issue reads `Filed 2026-09-17, open` or `Closed as not planned`.

## Step 4, the second table

| Item | What it is | Pending | Whose move |

**Pending** is one of a fixed set, so the column can be scanned rather than read. Decision,
Issue, Cleanup PR, Follow-up PR, Save tool, Review reply, Rebase, Verify.

**Whose move** is You or Me. This column is the point of the table. A decision is the user's
move and a cleanup PR is the agent's, and separating them turns the table into something
the user can act on line by line instead of a list to think about. When a user last got
this table they answered by settling three decisions and delegating the fourth item in a
single message.

**Keep it to what needs a person.** A row earns its place when someone has to decide
something, or when real work is still outstanding. Local housekeeping never qualifies.
Worktrees to delete, stashes to drop, build trees, patch files, logs and intermediate outputs
are the agent's own mess, so clean them up or leave them alone, but do not spend a row asking
about them. A scratch file earns a row only when it is a reusable tool, meaning the next
session would otherwise build it again from nothing, and even then the right move is to save
it and report it as done.

**Do it rather than table it.** A `Me` row that could be finished right now should be
finished right now, and reported in the first table instead.

Five rows is already a long list. When there are more, the extras are usually housekeeping,
or several steps of one larger item that should be a single row.

Sort so the user's rows come first. If the second table is empty, say so in one line rather
than printing an empty table.

Every row must trace to this session's own work. Stranded commits and dirty files in a
worktree another agent is using are that agent's business, and listing them hands the user
someone else's unfinished work as if it were theirs to decide on.

## When the conversation does not have the answers

A compacted session keeps its full history on disk. One session is one file that survives
resumes and compactions, at `~/.claude/projects/<slug>/<sessionId>.jsonl`, with the
subagents it spawned beside it under `<sessionId>/subagents/`. That is what `session_prs.py`
reads, so work that has fallen out of context is still recoverable. Do not delegate this to
a subagent. A scoped read is a few hundredths of a second, and a subagent would have to be
told the session id anyway to avoid sweeping every other agent on the machine.

A fresh agent pointed at a handoff document is the other case. Read the handoff in full,
then run the gather script anyway, because the handoff was written at some past moment and
the tables must be current. Where the two disagree the live query wins, and the disagreement
is worth a line of its own.

## Build the tables here, not in a subagent

Do this work in the main session. The tables are the deliverable and a subagent's report is
not shown to the user, so a delegated table has to be relayed back through the main context
anyway. There is no context saved and there is one more chance to garble it. The gathering
is a single script call, which is not the kind of noisy sweep that earns a subagent.

A fork does work, and it inherits the full conversation including tool results that were
never restated in prose. It is also the wrong tool here. A fork pays for a complete copy of
the parent context before it does anything, which measured at roughly 79k tokens on a short
session for a probe that made two tool calls, and that bill grows with the conversation and
recurs every time the user asks. Reach for a fork only when the main session is nearly out
of context and the summary is the last thing it has to do.

## Traps

Issues and pull requests share one number space in a repository, so `#1301` could be either
and `gh pr view` on an issue only errors. The scripts settle it through
`gh api repos/<owner>/<name>/issues/<n>`, which answers for both and flags a pull request.

A squash merge leaves no trace on the branch, so a branch that looks unmerged may be fully
landed. Confirm by pull request state or by content, never by `git branch --merged` alone.

A branch may track master rather than its own remote branch. Counting commits against
`@{u}` then reports merged work as unpushed. The script counts against all remotes instead,
which is why its unpushed lines can be trusted.

A conflicting pull request queues no checks at all, because there is no merge ref to run
them against. That reads as a stuck CI queue and is not one. The script says `CONFLICTING`
for it, and the disposition is Rebase.

Where a fork and an upstream both exist, the fork usually lags. Judge merged against the
upstream repository.

A session can move between worktrees while it runs, so the current shell directory is not
the scope. The scripts take the directories from the transcript instead.

Transcripts hold raw shell commands, and older ones carry access tokens inline. Never print
a command line out of a transcript. The scripts emit numbers and paths only.

Do not publish an artifact unless asked. The answer belongs in the terminal. When the user
wants something shareable and durable for a project spanning many workstreams, that is the
`project-artifact` skill rather than this one.
