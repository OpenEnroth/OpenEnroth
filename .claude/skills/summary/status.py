#!/usr/bin/env python3
"""Print the live state of pull requests and issues for the summary skill, one block per number.

usage: status.py [--repo owner/name] NUMBER...
       status.py [--repo owner/name] --candidates
"""
import argparse
import collections
import glob
import json
import os
import re
import subprocess
import sys

LINK = re.compile(r"github\.com/([\w.-]+)/([\w.-]+)/(?:pull|issues)/(\d+)")
GREEN = {"SUCCESS", "NEUTRAL", "SKIPPED"}
RED = {"FAILURE", "TIMED_OUT", "ACTION_REQUIRED", "ERROR", "STARTUP_FAILURE"}
THREADS = ("query($o:String!,$n:String!,$p:Int!){repository(owner:$o,name:$n){"
           "pullRequest(number:$p){reviewThreads(first:100){nodes{isResolved}}}}}")


def gh_json(*args):
    """Run gh and parse what it prints.

    @param args                         Arguments after `gh`.
    @return                             The decoded JSON output.
    """
    out = subprocess.run(["gh", *args], capture_output=True, text=True)
    if out.returncode != 0:
        raise RuntimeError((out.stderr.strip() or "gh failed").splitlines()[0])
    return json.loads(out.stdout)


def checks_phrase(pr):
    """Sum up a pull request's checks.

    @param pr                           Output of `gh pr view --json mergeable,statusCheckRollup`.
    @return                             Phrase such as "19/21 checks green, 2 pending or cancelled".
    """
    rollup = pr["statusCheckRollup"] or []
    verdicts = [(c.get("conclusion") or c.get("state") or "").upper() for c in rollup]  # A check run has a conclusion, a commit status has a state.
    green = sum(v in GREEN for v in verdicts)
    failed = [c.get("name") or c.get("context") for c, v in zip(rollup, verdicts) if v in RED]
    text = "%d/%d checks green" % (green, len(rollup)) if rollup else "no checks yet"
    if failed:
        text += ", failing: " + ", ".join(failed)
    if len(rollup) - green - len(failed):
        text += ", %d pending or cancelled" % (len(rollup) - green - len(failed))
    return "CONFLICTING, " + text if pr["mergeable"] == "CONFLICTING" else text


def unresolved_threads(repo, number):
    owner, name = repo.split("/")
    data = gh_json("api", "graphql", "-f", "query=" + THREADS, "-F", "o=" + owner, "-F", "n=" + name, "-F", "p=%d" % number)
    return sum(not t["isResolved"] for t in data["data"]["repository"]["pullRequest"]["reviewThreads"]["nodes"])


def print_item(repo, number):
    item = gh_json("api", "repos/%s/issues/%d" % (repo, number))  # This endpoint answers for pull requests too.
    kind = "PR" if "pull_request" in item else "Issue"
    if kind == "PR":
        pr = gh_json("pr", "view", str(number), "--repo", repo, "--json",
                     "state,isDraft,mergedAt,mergeable,reviewDecision,statusCheckRollup")
        if pr["state"] == "MERGED":
            status = ["Merged " + pr["mergedAt"][:10]]
        elif pr["state"] == "CLOSED":
            status = ["Closed unmerged"]
        else:
            status = ["Draft" if pr["isDraft"] else "Open", checks_phrase(pr)]
            if pr["reviewDecision"]:
                status.append(pr["reviewDecision"].replace("_", " ").lower())
            threads = unresolved_threads(repo, number)
            if threads:
                status.append("%d unresolved review threads" % threads)
    else:
        reason = (item.get("state_reason") or "completed").replace("_", " ")
        status = ["Open" if item["state"] == "open" else "Closed as " + reason, "filed " + item["created_at"][:10]]
    labels = [label["name"] for label in item["labels"]]
    if labels:
        status.append("labels: " + ", ".join(labels))

    body = re.sub(r"<!--.*?-->|```.*?```", " ", item["body"] or "", flags=re.S)
    body = re.sub(r"\s+", " ", body).strip()
    print("%s #%d %s" % (kind, number, item["title"]))
    print("  status: " + ", ".join(status))
    if body:
        print("  about: " + (body if len(body) <= 260 else body[:260].rstrip() + "..."))
    print("  cell: [%s #%d](%s)" % (kind, number, item["html_url"]))


def print_candidates(repo, session):
    """List every number this session's transcripts link to, most linked first.

    Only numbers are printed, because transcripts hold raw commands and some of those carry tokens.

    @param repo                         Owner/name whose links count.
    @param session                      Session id, which names its transcript and its subagents' directory.
    """
    projects = os.path.expanduser("~/.claude/projects")
    paths = glob.glob(os.path.join(projects, "*", session + ".jsonl"))
    paths += glob.glob(os.path.join(projects, "*", session, "subagents", "*.jsonl"))
    if not paths:
        raise RuntimeError("no transcript for session " + session)
    links = collections.Counter()
    for path in paths:
        with open(path, errors="replace") as f:
            for owner, name, number in LINK.findall(f.read()):
                if ("%s/%s" % (owner, name)).lower() == repo.lower():
                    links[int(number)] += 1
    for number, count in links.most_common():
        print("#%d  %d links" % (number, count))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--repo", help="owner/name, defaults to the repository of the current directory")
    ap.add_argument("--candidates", action="store_true", help="list the numbers this session's transcripts link to")
    ap.add_argument("numbers", nargs="*", type=int)
    args = ap.parse_args()
    if args.candidates == bool(args.numbers):
        ap.error("pass either numbers or --candidates")
    repo = args.repo or gh_json("repo", "view", "--json", "nameWithOwner")["nameWithOwner"]
    if args.candidates:
        if not os.environ.get("CLAUDE_CODE_SESSION_ID"):
            ap.error("--candidates needs $CLAUDE_CODE_SESSION_ID")
        print_candidates(repo, os.environ["CLAUDE_CODE_SESSION_ID"])
        return 0
    failed = False
    for number in args.numbers:
        try:
            print_item(repo, number)
        except (RuntimeError, KeyError, TypeError, ValueError) as e:
            print("#%d lookup failed: %s" % (number, e))
            failed = True
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
