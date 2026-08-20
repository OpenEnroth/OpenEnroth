You *MUST* read `HACKING.md` before doing any changes in this repo. You *MUST* follow the guidelines in `HACKING.md`, consider it a part of this document.

Build `check_style` and `check_tidy` targets to check style. You *MUST* always check style after your changes.

Keep comments terse - prefer a single trailing comment over a multi-line block, and never write comments explaining what you *didn't* do.

Build `Run_UnitTest` and `Run_GameTest_Headless_Parallel` targets to test your changes. You *MUST* always run tests after your changes. If you can't find the game data - ask the user to help you locate it, *NEVER* silently skip game tests.

*NEVER* use semicolons in prose - in comments, commit messages, PR descriptions or documentation. Write two sentences instead.

Comments in production code should say how the present code works, and only where the workings are non-trivial. *NEVER* narrate past bugs there - the reader needs the current invariant, not the history.

Comments in tests are the opposite. State the bug that the test guards against, otherwise it's unclear why the test exists, and someone will eventually delete it as redundant.

When you do write about a past bug, spell out that it *was* a bug and name it. "We used to keep the old buffer" reads like a reasonable choice that happened to change. "This used to be a heap buffer overflow" doesn't.

Put the `🤖 Human Needed` label on every pull request you open, once its CI is green. The label means a human has to look at the PR, and it's the only label you may ever add or remove - every other label belongs to the humans. Don't add it to a pull request that already carries any other label, a PR that a human has already triaged is in their pipeline anyway. Take the label off while you're working on review comments, and put it back when the ball is with the humans again.
