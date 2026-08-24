# Workflow

You *MUST* read `HACKING.md` before doing any changes in this repo. You *MUST* follow the guidelines in `HACKING.md`, consider it a part of this document.

Build `check_style` target to check style. You *MUST* always check style after your changes.

Build `Run_UnitTest` and `Run_GameTest_Headless_Parallel` targets to test your changes. You *MUST* always run tests after your changes. If you can't find the game data - ask the user to help you locate it, *NEVER* silently skip game tests.

Before every commit, re-read the diff's comments against the Comments section, as a separate pass with fresh eyes. You *MUST* do this - the review rounds exist for design questions, not for comment cleanup you could have caught yourself.

*NEVER* amend a commit or rewrite pushed history unless explicitly asked to. Fixes go on top as new commits with their own messages, and squashing is the human's call, made when they're ready.

Put the `🤖 Human Needed` label on every pull request you open, once its CI is green. The label means a human has to look at the PR, and it's the only label you may ever add or remove - every other label belongs to the humans. Don't add it to a pull request that already carries any other label, a PR that a human has already triaged is in their pipeline anyway. Take the label off while you're working on review comments, and put it back when the ball is with the humans again.


# Comments

Keep comments terse - prefer a single trailing comment over a multi-line block, and never write comments explaining what you *didn't* do.

A comment that explains a statement sits on that statement, inside the function. If it's about returning, it sits where the function returns. If it's about when to call, it sits at the call site. The doxygen block above a function is for callers only - what it does, what goes in, what comes out - never a walkthrough of the body.

Explanations live where the logic lives. Why a detection works goes next to the detection, the bug a test guards goes on the test, and a helper that just crashes needs no essay.

Every sentence in a comment must say something the code, the names, or the previous sentence don't already say. No pointers to other comments - state the fact or delete the sentence. If a call site needs a comment to be readable, fix the code instead - an enum parameter reads at the call site, a bool doesn't.

Any comment on a function, class, struct or table that's longer than a trailing one-liner is a doxygen block. Every doxygen block on a function *MUST* carry a `@param` tag for each parameter and a `@return` tag if the function returns something. A prose-only block isn't finished. Descriptions start at column 41, like the rest of the codebase.

An invariant is an assert, not a comment. If you're about to write a comment saying what must be true at this point in the code, write an `assert` instead.

Comments in production code should say how the present code works, and only where the workings are non-trivial. *NEVER* narrate past bugs there - the reader needs the current invariant, not the history.

Comments in tests are the opposite. State the bug that the test guards against, otherwise it's unclear why the test exists, and someone will eventually delete it as redundant.

When you do write about a past bug, spell out that it *was* a bug and name it. "We used to keep the old buffer" reads like a reasonable choice that happened to change. "This used to be a heap buffer overflow" doesn't.

*NEVER* use semicolons in prose - in comments, commit messages, PR descriptions or documentation. Write two sentences instead.

*NEVER* claim anything about game data or runtime behaviour that you haven't checked. OpenEnroth targets MM6, MM7 and MM8, so a claim about the data means all three were scanned. Go the extra mile when scanning data, and say what you checked - "no shipped record does this" means MM6, MM7 and MM8 scripts were all read, and if one of them wasn't, say which.
