#!/usr/bin/env python3
"""Find the pull requests and issues belonging to one session, and only that session.

A glob over ~/.claude/projects sweeps every session on the machine, which in a
worktree-per-task workflow means every parallel agent's work lands in the summary. One
session is one file named after its id, so the id is the scope.

Output is numbers and counts. Transcripts hold raw shell commands, some of them carrying
access tokens, so nothing from a command line is ever printed here.
"""
import argparse
import collections
import glob
import json
import os
import re
import sys

PROJECTS = os.path.expanduser("~/.claude/projects")

PULL_URL_RE = re.compile(r"github\.com/([\w.-]+)/([\w.-]+)/pull/(\d+)")
ISSUE_URL_RE = re.compile(r"github\.com/([\w.-]+)/([\w.-]+)/issues/(\d+)")
HASH_RE = re.compile(r"#(\d{2,6})\b")
PR_OP_RE = re.compile(r"\bgh\s+pr\s+(?:view|checks|merge|comment|edit|close|ready|diff|review)\s+(\d+)")
ISSUE_OP_RE = re.compile(r"\bgh\s+issue\s+(?:view|comment|edit|close|reopen|develop|pin)\s+(\d+)")
PR_CREATE_RE = re.compile(r"\bgh\s+pr\s+create\b")
ISSUE_CREATE_RE = re.compile(r"\bgh\s+issue\s+create\b")
API_RE = re.compile(r"\bgh\s+api\s+repos/[\w.-]+/[\w.-]+/(issues|pulls)/(\d+)")
# A command that searches for the text of a create call is not a create call. Grepping the
# transcripts for "gh issue create" otherwise files every link in the results as this
# session's own work.
SEARCHY_RE = re.compile(r"\b(grep|rg|ripgrep|ag|ack|fgrep|egrep)\b")

# Ranked weakest to strongest, so a stronger sighting always wins.
EVIDENCE = ("mentioned", "operated", "created")


def new_entry():
    """Build the record kept for one number.

    @return                             Fresh entry with no sightings and no kind.
    """
    return {"count": 0, "evidence": "mentioned", "first": "", "last": "", "kind": ""}


def transcript_paths(session_id):
    """Locate a session's own transcript and those of the subagents it spawned.

    @param session_id                   The session's id, which is also its file name.
    @return                             Tuple of the main transcript paths and the subagent ones.
    """
    main = glob.glob(os.path.join(PROJECTS, "*", session_id + ".jsonl"))
    subs = glob.glob(os.path.join(PROJECTS, "*", session_id, "subagents", "*.jsonl"))
    return main, subs


def blocks(rec):
    """Walk one record's tool calls, tool results and prose.

    @param rec                          One decoded transcript record.
    @return                             Yields tuples of kind, text and tool id.
    """
    msg = rec.get("message") or {}
    content = msg.get("content")
    if isinstance(content, str):
        yield "text", content, None
        return
    if not isinstance(content, list):
        return
    for b in content:
        if not isinstance(b, dict):
            continue
        kind = b.get("type")
        if kind == "tool_use":
            yield "tool_use", json.dumps(b.get("input") or {}), b.get("id")
        elif kind == "tool_result":
            yield "tool_result", json.dumps(b.get("content")), b.get("tool_use_id")
        elif kind == "text":
            yield "text", b.get("text") or "", None


def scan(paths, repo_filter, found, pending, places=None):
    """Walk transcripts and record every pull request and issue number sighted.

    `pending` holds the kind a `gh ... create` call is waiting on and the id of that call,
    because the new number only arrives in a later tool result. Pairing on the id matters.
    Without it a command that merely contains the text of a create call, such as a grep
    for one, arms the flag and the next link to scroll past is recorded as something this
    session filed.

    @param paths                        Transcript files to read.
    @param repo_filter                  Owner/name to keep, or None to keep every repository.
    @param found                        Mapping of number to entry, updated in place.
    @param pending                      One element list holding the armed create call, updated in place.
    @param places                       Optional dict collecting cwds, branches and the time window.
    @return                             Nothing, the results land in `found` and in `places`.
    """
    for path in paths:
        try:
            fh = open(path, errors="replace")
        except OSError:
            continue
        with fh:
            for line in fh:
                line = line.strip()
                if not line:
                    continue
                try:
                    rec = json.loads(line)
                except json.JSONDecodeError:
                    continue
                ts = (rec.get("timestamp") or "")[:19]
                if places is not None:
                    if rec.get("cwd"):
                        places["cwds"].add(rec["cwd"])
                    if rec.get("gitBranch"):
                        places["branches"].add(rec["gitBranch"])
                    if ts:
                        places["first"] = min(places["first"] or ts, ts)
                        places["last"] = max(places["last"] or ts, ts)

                for kind, text, tool_id in blocks(rec):
                    if not text:
                        continue
                    if kind == "tool_use" and tool_id and not SEARCHY_RE.search(text):
                        if PR_CREATE_RE.search(text):
                            pending[0] = ("pr", tool_id)
                        elif ISSUE_CREATE_RE.search(text):
                            pending[0] = ("issue", tool_id)

                    # A number carries only the evidence of its own match, because one block
                    # can hold both a number named by a gh call and plain references to others.
                    strong = {}
                    for rx, k in ((PULL_URL_RE, "pr"), (ISSUE_URL_RE, "issue")):
                        for owner, repo, num in rx.findall(text):
                            if repo_filter and "%s/%s" % (owner, repo) != repo_filter:
                                continue
                            strong[int(num)] = k
                    for num in PR_OP_RE.findall(text):
                        strong[int(num)] = "pr"
                    for num in ISSUE_OP_RE.findall(text):
                        strong[int(num)] = "issue"
                    for path_kind, num in API_RE.findall(text):
                        # The issues endpoint serves pull requests too, so it settles nothing.
                        strong.setdefault(int(num), "pr" if path_kind == "pulls" else "")
                    weak = set()
                    if kind in ("text", "tool_result"):
                        weak = {int(n) for n in HASH_RE.findall(text)} - set(strong)

                    for num in list(strong) + list(weak):
                        # Acting on something means having run a command that named it. A
                        # number that only appears in command output is incidental, which
                        # keeps a search result full of links from reading as this session's work.
                        rec_ev = "mentioned"
                        if num in strong and kind == "tool_use":
                            rec_ev = "operated"
                        if (pending[0] and kind == "tool_result" and num in strong
                                and tool_id == pending[0][1]):
                            rec_ev = "created"
                            if not found[num]["kind"]:
                                found[num]["kind"] = pending[0][0]
                            pending[0] = None
                        entry = found[num]
                        entry["count"] += 1
                        if EVIDENCE.index(rec_ev) > EVIDENCE.index(entry["evidence"]):
                            entry["evidence"] = rec_ev
                        if num in strong and not entry["kind"] and strong[num]:
                            entry["kind"] = strong[num]
                        if ts:
                            entry["first"] = min(entry["first"] or ts, ts)
                            entry["last"] = max(entry["last"] or ts, ts)


def session_scope(session_id, repo_filter=None):
    """Gather everything one session touched.

    The cwds and branches come from the per-record fields rather than from the current
    shell, because a session can move between worktrees while it runs. The window is what
    a search for issues filed during the session needs, to catch one opened in a browser
    where no transcript sees it.

    @param session_id                   The session to read.
    @param repo_filter                  Owner/name to keep, or None to keep every repository.
    @return                             Tuple of items, cwds, branches and the time window.
    """
    main_paths, sub_paths = transcript_paths(session_id)
    if not main_paths:
        return None, set(), set(), ("", "")
    found = collections.defaultdict(new_entry)
    places = {"cwds": set(), "branches": set(), "first": "", "last": ""}
    scan(main_paths, repo_filter, found, [None], places)
    scan(sub_paths, repo_filter, found, [None], places)
    return dict(found), places["cwds"], places["branches"], (places["first"], places["last"])


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--session", default=os.environ.get("CLAUDE_CODE_SESSION_ID"),
                    help="session id, defaults to $CLAUDE_CODE_SESSION_ID")
    ap.add_argument("--repo", default=None,
                    help="owner/name, drops links to any other repository")
    ap.add_argument("--min-count", type=int, default=2,
                    help="hide numbers seen fewer times than this unless the evidence is strong")
    args = ap.parse_args()

    if not args.session:
        print("No session id. Pass --session, or run where $CLAUDE_CODE_SESSION_ID is set.",
              file=sys.stderr)
        return 2

    main_paths, sub_paths = transcript_paths(args.session)
    if not main_paths:
        print("No transcript found for session %s under %s" % (args.session, PROJECTS),
              file=sys.stderr)
        return 2

    found, cwds, branches, window = session_scope(args.session, args.repo)

    print("# Pull requests and issues seen in session %s" % args.session)
    print("transcript: %s" % main_paths[0])
    print("subagent transcripts: %d" % len(sub_paths))
    print("worked in: %s" % (", ".join(sorted(cwds)) or "unknown"))
    print("branches seen: %s" % (", ".join(sorted(b for b in branches if b)) or "unknown"))
    print("session window: %s .. %s" % (window[0] or "?", window[1] or "?"))
    print()
    if not found:
        print("No pull request or issue numbers appear in this session.")
        return 0

    rows = sorted(found.items(),
                  key=lambda kv: (EVIDENCE.index(kv[1]["evidence"]), kv[1]["count"]),
                  reverse=True)
    print("%-8s %-7s %-10s %-9s %s" % ("NUMBER", "kind", "evidence", "sightings",
                                       "first seen .. last seen"))
    keep = []
    for num, e in rows:
        if e["evidence"] == "mentioned" and e["count"] < args.min_count:
            continue
        keep.append(num)
        print("%-8s %-7s %-10s %-9d %s .. %s" % ("#%d" % num, e["kind"] or "?", e["evidence"],
                                                 e["count"], e["first"] or "?", e["last"] or "?"))
    print()
    print("Feed the ones this session actually worked on into gather.py:")
    print("  " + " ".join("--pr %d" % n for n in keep[:20]))
    print()
    print("`operated` means the session ran a command naming it and `created` means it filed")
    print("it. `mentioned` is everything else, including links that came back inside command")
    print("output, so treat those as incidental until the conversation says otherwise. A bare")
    print("number can be a pull request or an issue, and gather.py resolves that on GitHub.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
