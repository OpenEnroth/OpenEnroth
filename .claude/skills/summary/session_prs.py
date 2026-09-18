#!/usr/bin/env python3
"""Find what one session worked on, and only that session.

One session is one transcript file named after its id, with the subagents it spawned in a
directory beside it, so the id is the scope. A glob over every transcript would report every
parallel agent's work as this session's.

Output is numbers, paths and counts. Transcripts hold raw shell commands, some of them carrying
access tokens, so no command text is ever printed.
"""
import argparse
import collections
import glob
import json
import os
import re
import sys

PROJECTS = os.path.expanduser("~/.claude/projects")

# Ranked weakest to strongest, so a stronger sighting always wins.
EVIDENCE = ("mentioned", "viewed", "changed", "created")

WRITE_VERBS = frozenset(("comment", "edit", "merge", "close", "ready", "review", "reopen", "lock",
                         "unlock", "pin", "unpin", "develop", "transfer", "delete"))
READ_VERBS = frozenset(("view", "checks", "diff", "checkout"))
EDIT_TOOLS = frozenset(("Edit", "Write", "NotebookEdit"))

ITEM_URL_RE = re.compile(r"github\.com/([\w.-]+)/([\w.-]+)/(pull|issues)/(\d+)")
API_PATH_RE = re.compile(r"repos/([\w.-]+)/([\w.-]+)/(issues|pulls)/(\d+)")
API_WRITE_RE = re.compile(r"(?:-X|--method)[ =]?(?:POST|PATCH|PUT|DELETE)\b|(?:^|\s)(?:-f|-F|--field|--raw-field|--input)\b")
REPO_FLAG_RE = re.compile(r"(?:^|\s)(?:-R|--repo)[ =]([\w.-]+/[\w.-]+)")
NUMBER_ARG_RE = re.compile(r"#?(\d{1,7})(?!\S)")
PREFIX_RE = re.compile(r"^(?:(?:[A-Za-z_]\w*=(?:\"[^\"]*\"|'[^']*'|\S*)|env|time|sudo|command|exec|nohup|setsid)\s+)*")
GIT_WRITE_RE = re.compile(r"git\s+(?:-C\s+(\S+)\s+)?(?:-c\s+\S+\s+)*(?:commit|push)\b")


def new_entry():
    """Build the record kept for one number.

    @return                             Fresh entry with no sightings and no kind.
    """
    return {"count": 0, "evidence": "mentioned", "first": "", "last": "", "kind": ""}


def new_places():
    """Build the record of where a session worked.

    @return                             Dict of path sets, one per kind of evidence.
    """
    return {"entered": set(), "committed": set(), "edited": set(), "subagent": set()}


def transcript_paths(session_id):
    """Locate a session's own transcript and those of the subagents it spawned.

    @param session_id                   The session's id, which is also its file name.
    @return                             Tuple of the main transcript paths and the subagent ones.
    """
    main = glob.glob(os.path.join(PROJECTS, "*", session_id + ".jsonl"))
    subs = glob.glob(os.path.join(PROJECTS, "*", session_id, "subagents", "*.jsonl"))
    return main, subs


def simple_commands(text):
    """Split shell text into simple commands, keeping quoted text inside its command.

    Matching on the split result is what keeps a search for the words of a command, or a
    command quoted inside a string, from reading as that command being run.

    @param text                         Raw command string from a Bash tool call.
    @return                             List of simple commands, each stripped.
    """
    text = text.replace("\\\n", " ")
    out, cur, quote = [], [], None
    i = 0
    while i < len(text):
        ch = text[i]
        if quote:
            cur.append(ch)
            if ch == "\\" and quote == '"' and i + 1 < len(text):
                i += 1
                cur.append(text[i])
            elif ch == quote:
                quote = None
        elif ch in "'\"":
            quote = ch
            cur.append(ch)
        elif ch in "\n;|&()":       # No backtick, which in a heredoc is far more often Markdown than a substitution.
            out.append("".join(cur))
            cur = []
        else:
            cur.append(ch)
        i += 1
    out.append("".join(cur))
    return [c.strip() for c in out if c.strip()]


def commands_in(text):
    """List every simple command in a shell string.

    The string is read twice, once whole and once a line at a time, and the results merged.
    The whole read follows quoting across lines. The line read still finds a command at the
    start of a line when an apostrophe in a heredoc body, or quotes nested inside a command
    substitution, have left the whole read inside a quote. Heredoc bodies are read as commands
    on purpose, because a session often writes a script that way and runs it in the same call.

    @param text                         Raw command string from a Bash tool call.
    @return                             List of simple commands with their leading assignments removed.
    """
    text = text.replace("\\\n", " ")
    seen = {}
    for chunk in simple_commands(text) + [c for line in text.split("\n") for c in simple_commands(line)]:
        cmd = PREFIX_RE.sub("", chunk)
        seen.setdefault(cmd.split("\n")[0][:160], cmd)
    return list(seen.values())


def same_repo(found, wanted):
    """Compare two owner/name strings the way GitHub does, ignoring case.

    @param found                        Repository named in the transcript.
    @param wanted                       Repository to keep, or None to keep every one.
    @return                             True when the sighting belongs to the wanted repository.
    """
    return not wanted or found.lower() == wanted.lower()


def result_text(block):
    """Pull the plain text out of a tool result block.

    @param block                        One tool_result content block.
    @return                             The result's text, empty when it holds none.
    """
    content = block.get("content")
    if isinstance(content, str):
        return content
    if isinstance(content, list):
        return "\n".join(p.get("text", "") for p in content if isinstance(p, dict) and p.get("type") == "text")
    return ""


class Scan:
    """One pass over a session's transcripts, collecting numbers and places."""

    def __init__(self, repo_filter):
        self.repo = repo_filter
        self.found = collections.defaultdict(new_entry)
        self.places = new_places()
        self._armed = set()             # Ids of create calls still waiting for their result.
        self._ts = ""

    def sight(self, num, evidence, kind):
        """Record one sighting of a number.

        @param num                      Pull request or issue number.
        @param evidence                 One of EVIDENCE.
        @param kind                     "pr", "issue", or empty when the sighting cannot tell.
        """
        entry = self.found[num]
        entry["count"] += 1
        if EVIDENCE.index(evidence) > EVIDENCE.index(entry["evidence"]):
            entry["evidence"] = evidence
        if kind == "pr" or (kind and not entry["kind"]):
            entry["kind"] = kind
        if self._ts:
            entry["first"] = min(entry["first"] or self._ts, self._ts)
            entry["last"] = max(entry["last"] or self._ts, self._ts)

    def mention(self, text):
        """Record every link to a pull request or issue in a piece of prose or output.

        @param text                     Assistant prose, user prose or command output.
        """
        for owner, name, path_kind, num in ITEM_URL_RE.findall(text):
            if same_repo("%s/%s" % (owner, name), self.repo):
                self.sight(int(num), "mentioned", "pr" if path_kind == "pull" else "issue")

    def gh_call(self, call, tool_id):
        """Classify one gh invocation.

        @param call                     A simple command that starts with gh.
        @param tool_id                  Id of the tool call it came from.
        """
        m = re.match(r"gh\s+(\S+)\s*(.*)$", call, re.S)
        if not m:
            return
        group, tail = m.group(1), m.group(2)
        flag = REPO_FLAG_RE.search(tail)
        if flag and not same_repo(flag.group(1), self.repo):
            return

        if group == "api":
            evidence = "changed" if API_WRITE_RE.search(tail) else "viewed"
            for owner, name, path_kind, num in API_PATH_RE.findall(tail):
                if same_repo("%s/%s" % (owner, name), self.repo):
                    # The issues endpoint serves pull requests too, so only pulls settles the kind.
                    self.sight(int(num), evidence, "pr" if path_kind == "pulls" else "")
            return

        if group not in ("pr", "issue"):
            return
        vm = re.match(r"(\S+)\s*(.*)$", tail, re.S)
        if not vm:
            return
        verb, args = vm.group(1), vm.group(2)
        if verb == "create":
            self._armed.add(tool_id)
            return
        if verb not in WRITE_VERBS and verb not in READ_VERBS:
            return
        evidence = "changed" if verb in WRITE_VERBS else "viewed"
        url = ITEM_URL_RE.match(re.sub(r"^https?://", "", args))
        if url:
            if same_repo("%s/%s" % (url.group(1), url.group(2)), self.repo):
                self.sight(int(url.group(4)), evidence, group)
            return
        num = NUMBER_ARG_RE.match(args)
        if num:
            self.sight(int(num.group(1)), evidence, group)

    def command(self, text, tool_id, cwd):
        """Read one Bash command for gh calls and for commits and pushes.

        @param text                     Raw command string.
        @param tool_id                  Id of the tool call.
        @param cwd                      Directory the session was in when it ran.
        """
        for cmd in commands_in(text):
            if cmd.startswith("gh "):
                self.gh_call(cmd, tool_id)
                continue
            git = GIT_WRITE_RE.match(cmd)
            if git:
                where = (git.group(1) or "").strip("'\"")
                where = where if where.startswith("/") else cwd
                if where:
                    self.places["committed"].add(where)

    def created(self, block):
        """Settle a create call from its own result.

        The new item is the last link gh prints, after any warning. A failed create prints the
        link of the item that already exists, so an errored result settles nothing.

        @param block                    The tool_result block answering an armed create call.
        """
        self._armed.discard(block.get("tool_use_id"))
        if block.get("is_error"):
            return
        links = [(o, n, k, num) for o, n, k, num in ITEM_URL_RE.findall(result_text(block))
                 if same_repo("%s/%s" % (o, n), self.repo)]
        if links:
            _, _, path_kind, num = links[-1]
            self.sight(int(num), "created", "pr" if path_kind == "pull" else "issue")

    def file(self, path, is_subagent):
        """Walk one transcript file.

        @param path                     Transcript to read.
        @param is_subagent              True for a subagent's transcript.
        """
        try:
            fh = open(path, errors="replace")
        except OSError:
            return
        with fh:
            for line in fh:
                line = line.strip()
                if not line:
                    continue
                try:
                    rec = json.loads(line)
                except json.JSONDecodeError:
                    continue
                self.record(rec, is_subagent)

    def record(self, rec, is_subagent):
        """Read one transcript record.

        @param rec                      Decoded record.
        @param is_subagent              True when it comes from a subagent's transcript.
        """
        if rec.get("type") == "worktree-state":
            entered = (rec.get("worktreeSession") or {}).get("worktreePath")
            if entered:
                self.places["entered"].add(entered)
            return
        self._ts = (rec.get("timestamp") or "")[:19]
        cwd = rec.get("cwd") or ""
        if is_subagent and cwd:
            self.places["subagent"].add(cwd)

        content = (rec.get("message") or {}).get("content")
        if isinstance(content, str):
            self.mention(content)
            return
        if not isinstance(content, list):
            return
        for block in content:
            if not isinstance(block, dict):
                continue
            kind = block.get("type")
            if kind == "text":
                self.mention(block.get("text") or "")
            elif kind == "tool_result":
                if block.get("tool_use_id") in self._armed:
                    self.created(block)
                self.mention(result_text(block))
            elif kind == "tool_use":
                inp = block.get("input")
                if not isinstance(inp, dict):
                    continue
                # Only a shell command acts on GitHub. The text of an edit, a written file or a
                # subagent prompt can quote any command without running it.
                if block.get("name") == "Bash" and isinstance(inp.get("command"), str):
                    self.command(inp["command"], block.get("id"), cwd)
                elif block.get("name") in EDIT_TOOLS:
                    target = inp.get("file_path") or inp.get("notebook_path")
                    if isinstance(target, str) and target.startswith("/"):
                        self.places["edited"].add(target)


def session_scope(session_id, repo_filter=None):
    """Gather everything one session touched.

    @param session_id                   The session to read.
    @param repo_filter                  Owner/name to keep, or None to keep every repository.
    @return                             Tuple of items and places, or (None, None) with no transcript.
    """
    main_paths, sub_paths = transcript_paths(session_id)
    if not main_paths:
        return None, None
    scan = Scan(repo_filter)
    for path in main_paths:
        scan.file(path, False)
    for path in sub_paths:
        scan.file(path, True)
    return dict(scan.found), scan.places


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--session", default=os.environ.get("CLAUDE_CODE_SESSION_ID"),
                    help="session id, defaults to $CLAUDE_CODE_SESSION_ID")
    ap.add_argument("--repo", default=None, help="owner/name, drops sightings in any other repository")
    ap.add_argument("--all", action="store_true", help="also list numbers the session only mentioned")
    args = ap.parse_args()

    if not args.session:
        print("No session id. Pass --session, or run where $CLAUDE_CODE_SESSION_ID is set.", file=sys.stderr)
        return 2
    main_paths, sub_paths = transcript_paths(args.session)
    found, places = session_scope(args.session, args.repo)
    if found is None:
        print("No transcript found for session %s under %s" % (args.session, PROJECTS), file=sys.stderr)
        return 2

    print("# Session %s" % args.session)
    print("transcript: %s" % main_paths[0])
    print("subagent transcripts: %d" % len(sub_paths))
    for tag in ("entered", "committed", "subagent"):
        if places[tag]:
            print("%s: %s" % (tag, ", ".join(sorted(places[tag]))))
    print("files edited: %d" % len(places["edited"]))
    print()

    rows = sorted(found.items(), key=lambda kv: (EVIDENCE.index(kv[1]["evidence"]), kv[1]["count"]), reverse=True)
    rows = [r for r in rows if args.all or r[1]["evidence"] != "mentioned"]
    if not rows:
        print("This session ran no gh command naming a pull request or an issue.")
        return 0
    print("%-8s %-6s %-10s %-9s %s" % ("NUMBER", "kind", "evidence", "sightings", "first seen .. last seen"))
    for num, e in rows:
        print("%-8s %-6s %-10s %-9d %s .. %s" % ("#%d" % num, e["kind"] or "?", e["evidence"], e["count"],
                                                 e["first"] or "?", e["last"] or "?"))
    print()
    print("created: a gh create call returned it. changed: the session ran a command that wrote to it.")
    print("viewed: the session only read it. A kind of ? gets settled by gather.py on GitHub.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
