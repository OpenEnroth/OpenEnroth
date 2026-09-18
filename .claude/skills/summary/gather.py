#!/usr/bin/env python3
"""Collect live state for one session's summary: its pull requests, issues and worktrees.

Every line printed comes from git and gh at the moment of the call. The session's transcript
decides which numbers and worktrees to ask about, and nothing else is taken from it.
"""
import argparse
import concurrent.futures
import json
import os
import re
import subprocess
import sys

sys.dont_write_bytecode = True          # A checked-in skill directory must not grow a __pycache__.
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from session_prs import session_scope  # noqa: E402

PR_FIELDS = "number,title,state,isDraft,mergedAt,closedAt,mergeable,reviewDecision,url,headRefName,labels,statusCheckRollup,body"
INDEX_FIELDS = "number,state,mergedAt,url,headRefName,isDraft"
INDEX_LIMIT = 200
NESTED_WORKTREES = ".claude/worktrees/"  # Where EnterWorktree and isolated subagents put theirs.
TRUNK_NAMES = frozenset(("master", "main", "HEAD"))

GREEN = frozenset(("SUCCESS", "NEUTRAL", "SKIPPED"))
RED = frozenset(("FAILURE", "TIMED_OUT", "ACTION_REQUIRED", "ERROR", "STARTUP_FAILURE"))
CANCELLED = frozenset(("CANCELLED", "STALE"))


def run(cmd, cwd=None, timeout=90):
    """Run a command and return its outcome, never raising.

    @param cmd                          Argument list to execute.
    @param cwd                          Directory to run it in, or None for this process's.
    @param timeout                      Seconds to wait before giving up.
    @return                             Tuple of exit code, stdout and stderr, all stripped.
    """
    try:
        p = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, timeout=timeout)
        return p.returncode, p.stdout.strip(), p.stderr.strip()
    except subprocess.TimeoutExpired:
        return 124, "", "timed out"
    except OSError as e:
        return 127, "", str(e)


def first_line(text):
    return text.splitlines()[0] if text else "no output"


def default_repo():
    """Resolve the repository from the current directory.

    @return                             Tuple of owner/name and an error, one of them empty.
    """
    code, out, err = run(["gh", "repo", "view", "--json", "nameWithOwner", "-q", ".nameWithOwner"])
    return (out, "") if code == 0 and out else ("", first_line(err))


def list_worktrees():
    """List the checkout's worktrees without touching any of them.

    @return                             List of dicts with path, branch and head, main checkout first.
    """
    code, out, _ = run(["git", "worktree", "list", "--porcelain"])
    if code != 0:
        return None
    trees, cur = [], None
    for line in out.splitlines():
        if line.startswith("worktree "):
            cur = {"path": line[9:], "branch": "", "head": "", "why": set()}
            trees.append(cur)
        elif cur is not None and line.startswith("branch "):
            cur["branch"] = line[7:].replace("refs/heads/", "")
        elif cur is not None and line.startswith("HEAD "):
            cur["head"] = line[5:][:11]
    return trees


def containing_tree(trees, path):
    """Find the worktree a path lies in.

    @param trees                        Worktrees from list_worktrees.
    @param path                         Absolute path of a file or directory.
    @return                             The deepest worktree holding the path, or None.
    """
    best = None
    for t in trees:
        if path == t["path"] or path.startswith(t["path"] + "/"):
            if best is None or len(t["path"]) > len(best["path"]):
                best = t
    return best


def session_trees(trees, places):
    """Pick the worktrees a session worked in, and say why each was picked.

    @param trees                        Worktrees from list_worktrees.
    @param places                       Evidence paths from session_scope.
    @return                             Tuple of the chosen worktrees and the names of ones since removed.
    """
    main = trees[0] if trees else None
    gone = set()
    for why in ("entered", "committed", "edited", "subagent"):
        for path in places[why]:
            tree = containing_tree(trees, path)
            if tree is None:
                continue
            if tree is main:
                inside = path[len(main["path"]) + 1:]
                if inside.startswith(NESTED_WORKTREES):
                    gone.add(inside[len(NESTED_WORKTREES):].split("/")[0])
                    continue
                # Every session starts in the main checkout, so only a commit or an edit there counts.
                if why not in ("committed", "edited"):
                    continue
            tree["why"].add(why)
    return [t for t in trees if t["why"]], sorted(gone)


def inspect_tree(tree):
    """Read one worktree's uncommitted files, unpublished commits and upstream branch.

    @param tree                         Worktree dict, updated in place.
    @return                             The same dict.
    """
    path = tree["path"]
    if not os.path.isdir(path):
        tree["error"] = "missing on disk"
        return tree
    # Without the flag, status refreshes the index and can hold index.lock while another agent commits there.
    code, out, err = run(["git", "--no-optional-locks", "status", "--porcelain"], cwd=path)
    if code != 0:
        tree["error"] = "git status failed: " + first_line(err)
        return tree
    tree["dirty"] = len([ln for ln in out.splitlines() if ln.strip()])
    code, out, _ = run(["git", "log", "--oneline", "HEAD", "--not", "--remotes"], cwd=path)
    rows = [ln for ln in out.splitlines() if ln.strip()] if code == 0 else []
    tree["unpublished"] = rows
    code, out, _ = run(["git", "rev-parse", "--abbrev-ref", "--symbolic-full-name", "@{u}"], cwd=path)
    upstream = out.split("/", 1)[1] if code == 0 and "/" in out else ""
    tree["upstream"] = "" if upstream in TRUNK_NAMES else upstream
    return tree


def pr_index(repo):
    """Map the head branch of every recent pull request to that pull request, in one call.

    @param repo                         Owner/name.
    @return                             Dict keyed by head branch, or None when the call failed.
    """
    code, out, _ = run(["gh", "pr", "list", "--repo", repo, "--state", "all", "--limit", str(INDEX_LIMIT),
                        "--json", INDEX_FIELDS], timeout=120)
    if code != 0:
        return None
    try:
        rows = json.loads(out)
    except json.JSONDecodeError:
        return None
    index = {}
    for row in rows:
        index.setdefault(row.get("headRefName"), row)
    return index


def item_view(repo, num):
    """Resolve a number through the issues endpoint, which answers for pull requests too.

    @param repo                         Owner/name.
    @param num                          The number to resolve.
    @return                             Normalised object with an is_pr flag, or one carrying an error key.
    """
    code, out, err = run(["gh", "api", "repos/%s/issues/%d" % (repo, num)])
    if code != 0:
        return {"number": num, "error": first_line(err)}
    try:
        d = json.loads(out)
    except json.JSONDecodeError:
        return {"number": num, "error": "unparseable gh output"}
    return {"number": d.get("number", num), "title": d.get("title", ""), "state": (d.get("state") or "").upper(),
            "state_reason": d.get("state_reason"), "url": d.get("html_url", ""), "body": d.get("body") or "",
            "labels": [lb.get("name") for lb in (d.get("labels") or [])], "createdAt": d.get("created_at", ""),
            "is_pr": "pull_request" in d}


def pr_view(repo, num):
    """Fetch one pull request with its checks, review state and description.

    @param repo                         Owner/name.
    @param num                          Pull request number.
    @return                             Pull request object, or one carrying an error key.
    """
    code, out, err = run(["gh", "pr", "view", str(num), "--repo", repo, "--json", PR_FIELDS])
    if code != 0:
        return {"number": num, "error": first_line(err)}
    try:
        return json.loads(out)
    except json.JSONDecodeError:
        return {"number": num, "error": "unparseable gh output"}


def unresolved_threads(repo, num):
    """Count open review threads through GraphQL, because REST counts resolved ones too.

    @param repo                         Owner/name.
    @param num                          Pull request number.
    @return                             Count of unresolved threads, or None when the query failed.
    """
    owner, name = repo.split("/", 1)
    query = ("query($o:String!,$n:String!,$p:Int!){repository(owner:$o,name:$n){"
             "pullRequest(number:$p){reviewThreads(first:100){nodes{isResolved}}}}}")
    code, out, _ = run(["gh", "api", "graphql", "-f", "query=" + query, "-F", "o=" + owner, "-F", "n=" + name,
                        "-F", "p=%d" % num])
    if code != 0:
        return None
    try:
        nodes = json.loads(out)["data"]["repository"]["pullRequest"]["reviewThreads"]["nodes"]
    except (KeyError, TypeError, json.JSONDecodeError):
        return None
    return sum(1 for node in nodes if not node.get("isResolved"))


def ci_summary(pr):
    """Roll an open pull request's checks into one phrase.

    @param pr                           Pull request object carrying statusCheckRollup.
    @return                             One phrase, empty for a pull request that is not open.
    """
    if pr.get("state") != "OPEN":
        return ""
    conflict = pr.get("mergeable") == "CONFLICTING"
    rollup = pr.get("statusCheckRollup") or []
    if not rollup:
        return "CONFLICTING, no merge ref so no checks queue" if conflict else "no checks reported yet"
    green, cancelled, running, failed = 0, 0, 0, []
    for check in rollup:
        # A check run reports a conclusion and a commit status reports a state. Anything unknown counts as running.
        verdict = (check.get("conclusion") or check.get("state") or "").upper()
        if verdict in GREEN:
            green += 1
        elif verdict in RED:
            failed.append("%s (%s)" % (check.get("name") or check.get("context") or "?", verdict.lower()))
        elif verdict in CANCELLED:
            cancelled += 1
        else:
            running += 1
    total = len(rollup)
    if green == total:
        text = "all %d checks green" % total
    else:
        parts = ["%d/%d green" % (green, total)]
        if failed:
            parts.append("%d FAILING: %s" % (len(failed), ", ".join(failed[:6])))
        if cancelled:
            parts.append("%d cancelled" % cancelled)
        if running:
            parts.append("%d still running" % running)
        text = ", ".join(parts)
    return "CONFLICTING, these checks predate the conflict. " + text if conflict else text


def about(item, limit=260):
    """Flatten the opening of an item's own description onto one line.

    @param item                         Pull request or issue object with a body.
    @param limit                        Characters to keep.
    @return                             One line, empty when there is no description.
    """
    body = re.sub(r"<!--.*?-->|```.*?```", " ", item.get("body") or "", flags=re.S)
    body = re.sub(r"\s+", " ", body).strip()
    return body if len(body) <= limit else body[:limit].rstrip() + "..."


def describe_pr(pr):
    """State one pull request's live status.

    @param pr                           Pull request object as gh returned it.
    @return                             One line naming state, checks, review and labels.
    """
    if pr.get("error"):
        return "lookup failed: " + pr["error"]
    state = pr.get("state")
    if state == "MERGED":
        bits = ["Merged %s" % (pr.get("mergedAt") or "")[:10]]
    elif state == "CLOSED":
        bits = ["Closed unmerged %s" % (pr.get("closedAt") or "")[:10]]
    else:
        bits = ["Draft" if pr.get("isDraft") else "Open", ci_summary(pr)]
        if pr.get("reviewDecision"):
            bits.append(pr["reviewDecision"].replace("_", " ").lower())
        threads = pr.get("_unresolved")
        if threads:
            bits.append("%d unresolved review thread%s" % (threads, "" if threads == 1 else "s"))
    labels = [lb["name"] for lb in (pr.get("labels") or [])]
    if labels:
        bits.append("labels: " + ", ".join(labels))
    return ". ".join(b for b in bits if b)


def describe_issue(issue):
    """State one issue's live status.

    @param issue                        Issue object from item_view.
    @return                             One line naming state, filing date and labels.
    """
    if issue.get("error"):
        return "lookup failed: " + issue["error"]
    if issue.get("state") == "CLOSED":
        bits = ["Closed as %s" % (issue.get("state_reason") or "completed").replace("_", " ")]
    else:
        bits = ["Open"]
    bits.append("filed %s" % issue.get("createdAt", "")[:10])
    if issue.get("labels"):
        bits.append("labels: " + ", ".join(issue["labels"]))
    return ". ".join(bits)


def short_state(row):
    """Phrase an indexed pull request's state for the end of a worktree row.

    @param row                          Pull request row from pr_index.
    @return                             One short phrase naming the number and the state.
    """
    if row.get("state") == "MERGED":
        return "PR #%d merged %s" % (row["number"], (row.get("mergedAt") or "")[:10])
    if row.get("state") == "CLOSED":
        return "PR #%d closed unmerged" % row["number"]
    return "PR #%d open%s" % (row["number"], ", draft" if row.get("isDraft") else "")


def print_trees(trees, index, hidden, scoped):
    """Print the worktree section.

    @param trees                        Inspected worktrees to show.
    @param index                        Branch to pull request map, or None when the lookup failed.
    @param hidden                       How many worktrees were left out.
    @param scoped                       True when the list is this session's only.
    @return                             Pull request numbers found on the shown worktrees' branches.
    """
    print("## Worktrees")
    if scoped:
        print("Only the ones this session entered, committed in or edited. %d other(s) left out." % hidden)
    else:
        print("The whole checkout, because --all-worktrees was passed. Most of these are other agents' work.")
    numbers = []
    for t in trees:
        label = t["branch"] or "detached at %s" % t["head"]
        if t.get("error"):
            print("- %s  [%s]  %s" % (t["path"], label, t["error"]))
            continue
        notes = []
        if t["dirty"]:
            notes.append("%d uncommitted file(s)" % t["dirty"])
        if t["unpublished"]:
            notes.append("%d commit(s) on no remote-tracking ref" % len(t["unpublished"]))
        row = None
        if index is None:
            notes.append("pull request lookup failed")
        elif t["branch"]:
            row = index.get(t["branch"]) or index.get(t["upstream"])
            notes.append(short_state(row) if row else "no PR under this branch name")
        if row:
            numbers.append(row["number"])
        why = ", ".join(sorted(t["why"])) if t["why"] else ""
        print("- %s  [%s]  %s%s" % (t["path"], label, ", ".join(notes) or "clean", "  (%s)" % why if why else ""))
        for line in t["unpublished"][:5]:
            print("      %s" % line)
    if any(t.get("unpublished") for t in trees):
        print("Commits pushed to a fork by URL, or to a remote nobody has fetched, also read as on no ref.")
    print()
    return numbers


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--session", default=os.environ.get("CLAUDE_CODE_SESSION_ID"),
                    help="session to summarise, defaults to $CLAUDE_CODE_SESSION_ID")
    ap.add_argument("--repo", default=None, help="owner/name, defaults to the repository of the current directory")
    ap.add_argument("--pr", action="append", default=[], type=int, metavar="N",
                    help="a pull request or issue the transcript missed, repeatable")
    ap.add_argument("--issue", action="append", default=[], type=int, metavar="N", help="same as --pr")
    ap.add_argument("--all-worktrees", action="store_true", help="show every worktree of the checkout")
    ap.add_argument("--no-worktrees", action="store_true", help="skip the worktree section")
    ap.add_argument("--no-threads", action="store_true", help="skip counting unresolved review threads")
    args = ap.parse_args()

    repo, err = (args.repo, "") if args.repo else default_repo()
    if not repo:
        print("Cannot tell which repository this is (%s). Run inside the checkout or pass --repo." % err)
        return 2
    print("# Live state for %s" % repo)

    items, places = session_scope(args.session, repo) if args.session else (None, None)
    scoped = items is not None
    if scoped:
        print("scope: session %s" % args.session)
    else:
        print("NO SESSION SCOPE. %s" % ("No transcript exists for session %s." % args.session if args.session
                                        else "$CLAUDE_CODE_SESSION_ID is not set and --session was not passed."))
        print("Nothing can be attributed to this session, so only numbers passed with --pr or --issue are shown.")
        items = {}
    print()

    tree_numbers = []
    if not args.no_worktrees and (scoped or args.all_worktrees):
        trees = list_worktrees()
        if trees is None:
            print("## Worktrees\nNot inside a git repository, so there are none to show.\n")
        else:
            chosen, gone = (trees, []) if args.all_worktrees else session_trees(trees, places)
            with concurrent.futures.ThreadPoolExecutor(max_workers=8) as pool:
                chosen = list(pool.map(inspect_tree, chosen))
            index = pr_index(repo) if any(t["branch"] for t in chosen) else {}
            tree_numbers = print_trees(chosen, index, len(trees) - len(chosen), not args.all_worktrees)
            if gone:
                print("Worked in but since removed: %s\n" % ", ".join(gone))

    owned = [n for n, e in items.items() if e["evidence"] in ("created", "changed")]
    wanted = list(dict.fromkeys(args.pr + args.issue + owned + tree_numbers))
    with concurrent.futures.ThreadPoolExecutor(max_workers=6) as pool:
        resolved = dict(zip(wanted, pool.map(lambda n: item_view(repo, n), wanted)))
        pr_numbers = [n for n in wanted if resolved[n].get("is_pr") or resolved[n].get("error")]
        prs = dict(zip(pr_numbers, pool.map(lambda n: pr_view(repo, n), pr_numbers)))
        open_prs = [] if args.no_threads else [n for n in pr_numbers if prs[n].get("state") == "OPEN"]
        for n, count in zip(open_prs, pool.map(lambda n: unresolved_threads(repo, n), open_prs)):
            prs[n]["_unresolved"] = count
    issues = {n: it for n, it in resolved.items() if n not in prs}

    if prs:
        print("## Pull requests")
        for n in sorted(prs, reverse=True):
            print("- #%d %s" % (n, prs[n].get("title", "")))
            print("    %s" % describe_pr(prs[n]))
            if about(prs[n]):
                print("    about: %s" % about(prs[n]))
            if prs[n].get("url"):
                print("    %s" % prs[n]["url"])
        print()
    if issues:
        print("## Issues")
        for n in sorted(issues, reverse=True):
            print("- #%d %s" % (n, issues[n].get("title", "")))
            print("    %s" % describe_issue(issues[n]))
            if about(issues[n]):
                print("    about: %s" % about(issues[n]))
            if issues[n].get("url"):
                print("    %s" % issues[n]["url"])
        print()
    if not prs and not issues:
        print("No pull requests or issues belong to this session.\n")

    viewed = sorted(n for n, e in items.items() if e["evidence"] == "viewed" and n not in wanted)
    if viewed:
        print("Only read by this session, so left out: %s" % " ".join("#%d" % n for n in viewed))
        print("If the session did work on one of them, run again with --pr N.\n")
    print("Every line above is a live query.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
