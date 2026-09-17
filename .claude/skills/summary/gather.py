#!/usr/bin/env python3
"""Collect live PR, branch and worktree state for a session summary.

Everything printed here comes from git and gh at the moment of the call. Nothing is
remembered, which is the whole point of the script.
"""
import argparse
import concurrent.futures
import json
import os
import re
import subprocess
import sys

# Importing a sibling would drop a __pycache__ next to it, and a repository that checks this
# skill in does not ignore one. The summary's own worktree sweep would then report the skill
# directory as uncommitted work.
sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
try:
    from session_prs import session_scope
except ImportError:
    session_scope = None


def run(cmd, cwd=None, timeout=60):
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
        return 124, "", "timeout"
    except OSError as e:
        return 127, "", str(e)


def default_repo():
    """Resolve the repository from the current directory.

    @return                             Owner/name string, empty when gh cannot tell.
    """
    code, out, _ = run(["gh", "repo", "view", "--json", "nameWithOwner", "-q", ".nameWithOwner"])
    return out if code == 0 and out else ""


def worktrees():
    """Survey every worktree of this checkout.

    @return                             List of dicts holding path, branch, dirt and unpushed commits.
    """
    code, out, _ = run(["git", "worktree", "list", "--porcelain"])
    if code != 0:
        return []
    trees, cur = [], {}
    for line in out.splitlines():
        if line.startswith("worktree "):
            if cur:
                trees.append(cur)
            cur = {"path": line[9:], "branch": None, "detached": False}
        elif line.startswith("branch "):
            cur["branch"] = line[7:].replace("refs/heads/", "")
        elif line.strip() == "detached":
            cur["detached"] = True
        elif line.startswith("HEAD "):
            cur["head"] = line[5:][:11]
    if cur:
        trees.append(cur)

    def enrich(t):
        path = t["path"]
        if not os.path.isdir(path):
            t["missing"] = True
            return t
        _, st, _ = run(["git", "status", "--porcelain"], cwd=path)
        t["dirty"] = len([l for l in st.splitlines() if l.strip()])
        # Commits on HEAD that no remote ref contains. Diffing against @{u} instead reports
        # a branch tracking master as having its own merged commits unpushed, which is a
        # false stranded-work alarm rather than a near miss.
        _, ahead, _ = run(["git", "log", "--oneline", "HEAD", "--not", "--remotes"], cwd=path)
        rows = [l for l in ahead.splitlines() if l.strip()]
        t["unpushed"] = len(rows)
        t["unpushed_subjects"] = rows[:5]
        _, contains, _ = run(["git", "branch", "-r", "--contains", "HEAD"], cwd=path)
        t["on_remote"] = [c.strip() for c in contains.splitlines() if c.strip()][:3]
        return t

    with concurrent.futures.ThreadPoolExecutor(max_workers=8) as ex:
        trees = list(ex.map(enrich, trees))
    return trees


PR_FIELDS = ("number,title,state,isDraft,mergedAt,closedAt,mergeable,reviewDecision,"
             "url,headRefName,labels,statusCheckRollup,body")


def pr_by_number(repo, num):
    """Fetch one pull request with the fields the tables need.

    @param repo                         Owner/name, or empty for the current repository.
    @param num                          Pull request number.
    @return                             Pull request object, or one carrying an error key.
    """
    cmd = ["gh", "pr", "view", str(num), "--json", PR_FIELDS]
    if repo:
        cmd += ["--repo", repo]
    code, out, err = run(cmd, timeout=90)
    if code != 0:
        return {"number": num, "error": err or "lookup failed"}
    try:
        return json.loads(out)
    except json.JSONDecodeError:
        return {"number": num, "error": "unparseable gh output"}


def pr_by_branch(repo, branch):
    """Find the pull requests opened from a branch.

    @param repo                         Owner/name, or empty for the current repository.
    @param branch                       Head branch name.
    @return                             List of pull request objects, empty when there are none or the call failed.
    """
    cmd = ["gh", "pr", "list", "--head", branch, "--state", "all", "--limit", "5",
           "--json", PR_FIELDS]
    if repo:
        cmd += ["--repo", repo]
    code, out, err = run(cmd, timeout=90)
    if code != 0:
        return []
    try:
        return json.loads(out)
    except json.JSONDecodeError:
        return []


def pr_index(repo, limit=200):
    """One call that maps every recent head branch to its pull request.

    Looking each branch up on its own costs a call per worktree, and a checkout with
    thirty worktrees then spends half a minute on it.

    @param repo                         Owner/name, or empty for the current repository.
    @param limit                        How many recent pull requests to index.
    @return                             Tuple of the branch to pull request map, and whether the limit was hit.
    """
    cmd = ["gh", "pr", "list", "--state", "all", "--limit", str(limit),
           "--json", "number,title,state,mergedAt,closedAt,url,headRefName,isDraft"]
    if repo:
        cmd += ["--repo", repo]
    code, out, _ = run(cmd, timeout=120)
    if code != 0:
        return {}, False
    try:
        rows = json.loads(out)
    except json.JSONDecodeError:
        return {}, False
    idx = {}
    for r in rows:
        idx.setdefault(r.get("headRefName"), r)
    return idx, len(rows) >= limit


def short_state(pr):
    """Phrase a pull request's state short enough to sit at the end of a branch row.

    @param pr                           Pull request object from the index.
    @return                             One short phrase naming the number and the state.
    """
    st = pr.get("state", "?")
    if st == "MERGED":
        return "PR #%d merged %s" % (pr["number"], (pr.get("mergedAt") or "")[:10])
    if st == "CLOSED":
        return "PR #%d closed unmerged" % pr["number"]
    return "PR #%d open%s" % (pr["number"], ", draft" if pr.get("isDraft") else "")


def unresolved_threads(repo, num):
    """Count open review threads, which only GraphQL reports correctly.

    REST miscounts them, because a resolved thread still carries its top level comment.

    @param repo                         Owner/name.
    @param num                          Pull request number.
    @return                             Count of unresolved threads, or None when the query failed.
    """
    if not repo or "/" not in repo:
        return None
    owner, name = repo.split("/", 1)
    q = ("query($o:String!,$n:String!,$p:Int!){repository(owner:$o,name:$n){"
         "pullRequest(number:$p){reviewThreads(first:100){nodes{isResolved isOutdated}}}}}")
    code, out, _ = run(["gh", "api", "graphql", "-f", "query=" + q,
                        "-F", "o=" + owner, "-F", "n=" + name, "-F", "p=%d" % num], timeout=60)
    if code != 0:
        return None
    try:
        nodes = json.loads(out)["data"]["repository"]["pullRequest"]["reviewThreads"]["nodes"]
    except (KeyError, TypeError, json.JSONDecodeError):
        return None
    return sum(1 for n in nodes if not n.get("isResolved"))


def ci_summary(pr):
    """Roll a pull request's check list into one phrase.

    @param pr                           Pull request object carrying statusCheckRollup.
    @return                             One phrase, empty for a pull request that is not open.
    """
    if pr.get("state") != "OPEN":
        return ""
    if pr.get("mergeable") == "CONFLICTING":
        return "CONFLICTING, no merge ref so no checks queue at all"
    rollup = pr.get("statusCheckRollup") or []
    if not rollup:
        return "no checks reported yet"
    done = fail = pending = 0
    failed_names = []
    for c in rollup:
        state = (c.get("conclusion") or c.get("state") or "").upper()
        status = (c.get("status") or "").upper()
        if state in ("SUCCESS", "NEUTRAL", "SKIPPED"):
            done += 1
        elif state in ("FAILURE", "TIMED_OUT", "ACTION_REQUIRED", "ERROR", "CANCELLED"):
            fail += 1
            failed_names.append("%s (%s)" % (c.get("name", "?"), state.lower()))
        elif status in ("QUEUED", "IN_PROGRESS", "PENDING", "WAITING") or not state:
            pending += 1
        else:
            done += 1
    total = len(rollup)
    if fail:
        return "%d/%d green, %d FAILING: %s" % (done, total, fail, ", ".join(failed_names[:6]))
    if pending:
        return "%d/%d green, %d still running" % (done, total, pending)
    return "all %d checks green" % total


def body_excerpt(pr, limit=260):
    """The opening of a pull request's own description, flattened onto one line.

    The summary has to say what a pull request is about, and a session that only pushed
    review fixes onto someone else's branch never learned that from its own work.

    @param pr                           Pull request object as gh returned it.
    @param limit                        Characters to keep before cutting the excerpt.
    @return                             One line of description, empty when there is no body.
    """
    body = (pr.get("body") or "").strip()
    if not body:
        return ""
    body = re.sub(r"<!--.*?-->", " ", body, flags=re.S)
    body = re.sub(r"```.*?```", " ", body, flags=re.S)
    body = re.sub(r"\s+", " ", body).strip()
    if len(body) <= limit:
        return body
    return body[:limit].rstrip() + "..."


def describe(pr):
    """State one pull request's live status.

    @param pr                           Pull request object as gh returned it.
    @return                             One line naming state, checks, review and labels.
    """
    if pr.get("error"):
        return "lookup failed: %s" % pr["error"]
    state = pr.get("state", "?")
    bits = []
    if state == "MERGED":
        bits.append("Merged %s" % (pr.get("mergedAt") or "")[:10])
    elif state == "CLOSED":
        bits.append("Closed unmerged %s" % (pr.get("closedAt") or "")[:10])
    else:
        bits.append("Draft" if pr.get("isDraft") else "Open")
        ci = ci_summary(pr)
        if ci:
            bits.append(ci)
        rd = pr.get("reviewDecision")
        if rd:
            bits.append(rd.replace("_", " ").lower())
        n = pr.get("_unresolved")
        if n:
            bits.append("%d unresolved review thread%s" % (n, "" if n == 1 else "s"))
    labels = [l["name"] for l in (pr.get("labels") or [])]
    if labels:
        bits.append("labels: " + ", ".join(labels))
    return ". ".join(bits)


def issue_view(repo, num):
    """Resolve a number through the issues endpoint, which answers for pull requests too.

    A bare #1234 in a transcript can be either, and `gh pr view` on an issue just errors,
    so the kind gets settled here rather than guessed from the conversation.

    @param repo                         Owner/name.
    @param num                          The number to resolve.
    @return                             Normalised object carrying an is_pr flag, or one carrying an error key.
    """
    if not repo:
        return None
    code, out, err = run(["gh", "api", "repos/%s/issues/%d" % (repo, num)], timeout=60)
    if code != 0:
        return {"number": num, "error": err.splitlines()[0] if err else "lookup failed"}
    try:
        d = json.loads(out)
    except json.JSONDecodeError:
        return {"number": num, "error": "unparseable gh output"}
    return {
        "number": d.get("number", num),
        "title": d.get("title", ""),
        "state": (d.get("state") or "").upper(),
        "state_reason": d.get("state_reason"),
        "url": d.get("html_url", ""),
        "labels": [l.get("name") for l in (d.get("labels") or [])],
        "createdAt": d.get("created_at", ""),
        "is_pr": "pull_request" in d,
    }


def describe_issue(it):
    """State one issue's live status.

    @param it                           Issue object from issue_view or from the window search.
    @return                             One line naming state, filing date and labels.
    """
    if it.get("error"):
        return "lookup failed: %s" % it["error"]
    bits = []
    if it.get("state") == "CLOSED":
        reason = it.get("state_reason") or "completed"
        bits.append("Closed as %s" % reason.replace("_", " "))
    else:
        bits.append("Open")
    if it.get("createdAt"):
        bits.append("filed %s" % it["createdAt"][:10])
    if it.get("labels"):
        bits.append("labels: " + ", ".join(it["labels"]))
    return ". ".join(bits)


def issues_in_window(repo, since, until, author):
    """Find the issues filed while the session ran, catching one opened outside the terminal.

    @param repo                         Owner/name.
    @param since                        Earliest timestamp to accept.
    @param until                        Latest timestamp to accept.
    @param author                       Login to filter by, or None for any.
    @return                             List of issue rows, empty when there are none or the call failed.
    """
    if not (repo and since):
        return []
    cmd = ["gh", "issue", "list", "--repo", repo, "--state", "all", "--limit", "60",
           "--search", "created:>=%s" % since[:10],
           "--json", "number,title,state,stateReason,createdAt,url,labels,author"]
    if author:
        cmd += ["--author", author]
    code, out, _ = run(cmd, timeout=90)
    if code != 0:
        return []
    try:
        rows = json.loads(out)
    except json.JSONDecodeError:
        return []
    hits = []
    for r in rows:
        created = (r.get("createdAt") or "")[:19]
        if since and created < since:
            continue
        if until and created > until:
            continue
        hits.append(r)
    return hits


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--repo", default=None, help="owner/name, defaults to the cwd repo")
    ap.add_argument("--pr", action="append", default=[], type=int,
                    help="a pull request this session touched, repeatable")
    ap.add_argument("--issue", action="append", default=[], type=int,
                    help="an issue this session touched, repeatable")
    ap.add_argument("--no-issues", action="store_true",
                    help="skip the search for issues filed while the session ran")
    ap.add_argument("--branch", action="append", default=[],
                    help="a branch this session touched, repeatable")
    ap.add_argument("--session", default=os.environ.get("CLAUDE_CODE_SESSION_ID"),
                    help="scope to this session, defaults to $CLAUDE_CODE_SESSION_ID")
    ap.add_argument("--all-worktrees", action="store_true",
                    help="report every worktree of the checkout, not just this session's")
    ap.add_argument("--no-worktrees", action="store_true",
                    help="skip the worktree sweep, which is the slow part")
    ap.add_argument("--threads", action="store_true",
                    help="also count unresolved review threads, one extra call per open PR")
    args = ap.parse_args()

    repo = args.repo or default_repo()
    print("# Live state")
    print("repo: %s" % (repo or "unknown"))
    code, who, _ = run(["gh", "api", "user", "-q", ".login"])
    gh_user = who if code == 0 else None
    print("gh user: %s" % (gh_user or "NOT LOGGED IN, run gh auth status"))
    print()

    index, truncated = pr_index(repo)

    # Scope to this session unless asked for the whole checkout. A worktree-per-task
    # workflow has dozens of worktrees and parallel agents, and sweeping all of them
    # reports other agents' work as this session's.
    scope_cwds, scope_branches, auto_prs, auto_kind = set(), set(), [], {}
    window = ("", "")
    scoped = False
    if args.session and session_scope and not args.all_worktrees:
        found, scope_cwds, scope_branches, window = session_scope(args.session, repo or None)
        if found is not None:
            scoped = True
            auto_prs = [n for n, e in found.items() if e["evidence"] in ("created", "operated")]
            auto_kind = {n: found[n].get("kind") for n in auto_prs}
        else:
            print("No transcript for session %s, falling back to the whole checkout." % args.session)
    if scoped:
        print("scope: session %s" % args.session)
        print("worked in: %s" % (", ".join(sorted(scope_cwds)) or "unknown"))
        print()

    def in_scope(t):
        if not scoped:
            return True
        if t.get("path") in scope_cwds:
            return True
        return bool(t.get("branch")) and t["branch"] in scope_branches

    if not args.no_worktrees:
        trees = worktrees()
        shown = [t for t in trees if in_scope(t)]
        hidden = len(trees) - len(shown)
        print("## Worktrees")
        if scoped:
            print("Only the worktrees this session worked in. %d other worktree(s) hidden," % hidden)
            print("pass --all-worktrees to see them.")
        else:
            print("Every worktree of this checkout, not only this session's. Treat a row you do")
            print("not recognise as someone else's work, but do read the unpushed lines.")
        for t in shown:
            if t.get("missing"):
                print("- %s  MISSING ON DISK" % t["path"])
                continue
            label = t.get("branch") or ("detached at %s" % t.get("head", "?"))
            notes = []
            if t.get("dirty"):
                notes.append("%d uncommitted file(s)" % t["dirty"])
            if t.get("unpushed"):
                notes.append("%d unpushed commit(s)" % t["unpushed"])
            if not t.get("on_remote") and not t.get("unpushed"):
                notes.append("no remote has this HEAD")
            pr = index.get(t.get("branch"))
            if pr:
                notes.append(short_state(pr))
            elif t.get("branch"):
                notes.append("no PR found")
            print("- %-58s %-42s %s" % (t["path"], label, ", ".join(notes) or "clean"))
            for sub in t.get("unpushed_subjects", []):
                print("      unpushed: %s" % sub)
        if truncated:
            print("(The PR index hit its limit, so a very old branch may read as \"no PR found\".)")
        print()

    prs, issues = {}, {}
    want = list(dict.fromkeys(list(args.pr) + list(args.issue) + auto_prs))
    # A number whose kind the transcript did not settle goes through the issues endpoint,
    # which reports pull requests too, so one call sorts it into the right table.
    kinds = dict(auto_kind)
    for n in args.pr:
        kinds[n] = "pr"
    for n in args.issue:
        kinds[n] = "issue"
    unknown = [n for n in want if kinds.get(n) not in ("pr", "issue")]
    if unknown:
        with concurrent.futures.ThreadPoolExecutor(max_workers=6) as ex:
            for n, res in zip(unknown, ex.map(lambda n: issue_view(repo, n), unknown)):
                if res and not res.get("error"):
                    kinds[n] = "pr" if res.get("is_pr") else "issue"
                    if not res.get("is_pr"):
                        issues[n] = res
                else:
                    kinds[n] = "pr"

    pr_nums = [n for n in want if kinds.get(n) == "pr"]
    issue_nums = [n for n in want if kinds.get(n) == "issue" and n not in issues]
    with concurrent.futures.ThreadPoolExecutor(max_workers=6) as ex:
        for pr in ex.map(lambda n: pr_by_number(repo, n), pr_nums):
            if pr.get("number"):
                prs[pr["number"]] = pr
        for it in ex.map(lambda n: issue_view(repo, n), issue_nums):
            if it and it.get("number"):
                issues[it["number"]] = it
        if args.branch:
            for lst in ex.map(lambda b: pr_by_branch(repo, b), args.branch):
                for pr in lst:
                    prs.setdefault(pr["number"], pr)

    if args.threads:
        open_prs = [p for p in prs.values() if p.get("state") == "OPEN"]
        with concurrent.futures.ThreadPoolExecutor(max_workers=6) as ex:
            counts = list(ex.map(lambda p: unresolved_threads(repo, p["number"]), open_prs))
        for pr, n in zip(open_prs, counts):
            pr["_unresolved"] = n

    if prs:
        print("## Pull requests")
        if auto_prs and not args.pr:
            print("Found in this session's transcript. A number the session only discussed can")
            print("still land here, so drop any row the conversation does not back up.")
        for num in sorted(prs, reverse=True):
            pr = prs[num]
            print("- #%d %s" % (num, pr.get("title", "")))
            print("    %s" % describe(pr))
            about = body_excerpt(pr)
            if about:
                print("    about: %s" % about)
            if pr.get("headRefName"):
                print("    branch: %s  %s" % (pr["headRefName"], pr.get("url", "")))
        print()

    # Issues opened while the session ran, which catches one filed outside the terminal.
    if scoped and not args.no_issues and window[0]:
        for r in issues_in_window(repo, window[0], window[1], gh_user):
            n = r.get("number")
            if n in issues or n in prs:
                continue
            issues[n] = {"number": n, "title": r.get("title", ""),
                         "state": (r.get("state") or "").upper(),
                         "state_reason": r.get("stateReason"),
                         "url": r.get("url", ""),
                         "labels": [l.get("name") for l in (r.get("labels") or [])],
                         "createdAt": r.get("createdAt", ""), "is_pr": False,
                         "_window": True}

    if issues:
        print("## Issues")
        print("Filed or worked on by this session.")
        for num in sorted(issues, reverse=True):
            it = issues[num]
            tag = " (filed during this session)" if it.get("_window") else ""
            print("- #%d %s%s" % (num, it.get("title", ""), tag))
            print("    %s" % describe_issue(it))
            if it.get("url"):
                print("    %s" % it["url"])
        print()

    asked = set(args.branch)
    unresolved_branches = [b for b in asked if b not in index and
                           not any(p.get("headRefName") == b for p in prs.values())]
    if unresolved_branches:
        print("## Named branches with no pull request found")
        for b in unresolved_branches:
            print("- %s" % b)
        print("A squash merge leaves no trace on the branch, so confirm by content before")
        print("calling one of these unmerged.")
        print()

    print("Gathered by gather.py. Every line above is a live query, none of it is remembered state.")


if __name__ == "__main__":
    sys.exit(main())
