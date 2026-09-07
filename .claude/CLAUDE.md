# General

*NEVER* use semicolons in prose - in comments, commit messages, PR descriptions or documentation. Write two sentences instead.

# Workflow

You *MUST* read `HACKING.md` before doing any changes in this repo. You *MUST* follow the guidelines in `HACKING.md`, consider it a part of this document.

Build `check_style` and `check_tidy` targets to check style. You *MUST* always check style after your changes.

Build `Run_UnitTest` and `Run_GameTest_Headless_Parallel` targets to test your changes. You *MUST* always run tests after your changes. If you can't find the game data - ask the user to help you locate it, *NEVER* silently skip game tests.

*NEVER* claim anything about game data or runtime behaviour that you haven't checked. OpenEnroth targets MM6, MM7 and MM8, so a claim about the data means all three were scanned. Go the extra mile when scanning data, and say what you checked - "no shipped record does this" means MM6, MM7 and MM8 scripts were all read, and if one of them wasn't, say which.

Before every commit, do a deletion pass over the comments in the diff, as a separate step. For each added comment ask whether it describes the code or defends the change, and delete the ones that defend it. You *MUST* do this, the review rounds exist for design questions, not for comment cleanup you could have caught yourself.

*NEVER* amend a commit or rewrite pushed history unless explicitly asked to. Fixes go on top as new commits with their own messages, and squashing is the human's call.

The commit message is where the reasoning lives. Say what was wrong and why this is the fix, in a paragraph. Every "why" you were tempted to put in a comment goes here instead.

Keep pull request descriptions short - a few sentences on what was done and how, and that's it. Two or three paragraphs at most, humans don't want to read an essay. The evidence you gathered along the way belongs in a comment on the PR if it belongs anywhere.

Put the `🤖 Human Needed` label on every pull request you open, once its CI is green. The label means a human has to look at the PR, and it's the only label you may ever add or remove - every other label belongs to the humans. Don't add it to a pull request that already carries any other label, a PR that a human has already triaged is in their pipeline anyway. Take the label off while you're working on review comments, and put it back when the ball is with the humans again.

# Comments

The default is no comment. Write one where the code can't say it, a non-obvious runtime or domain fact, a hidden dependency, a value that looks wrong and isn't. Most comments in this codebase are a single trailing line, match that.

A comment describes the code. It *NEVER* defends the change. If a sentence says why the diff is right, what would break without this line, or what the test would catch, it's commit message material and it doesn't go in the source. "Nothing else would notice", "this is what catches", "so that we" are the tells, delete the sentence.

Every sentence in a comment must say something the code, the names and the previous sentence don't. If a call site needs a comment to be readable, fix the code instead. An enum parameter reads at the call site. A bool doesn't.

Explanations live where the logic lives. A comment about a statement is a trailing comment on that statement. A comment about a return sits at the return. A comment about a call sits at the call. The doxygen block above a function is for callers only, what goes in and what comes out, never a walkthrough of the body.

Public functions, classes, structs and tables get a doxygen block, with a `@param` tag for each parameter and a `@return` tag if there's a result. Descriptions start at column 41. Helpers and test functions don't get one, a trailing line inside the body does the job.

An invariant is an assert, not a comment.

Production code never narrates past bugs. A test does the opposite, it says in one line which bug it guards against, and calls it a bug. "We used to keep the old buffer" reads like a choice. "This used to be a heap buffer overflow" doesn't.
