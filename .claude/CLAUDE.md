# General

*NEVER* use semicolons in prose - in comments, commit messages, PR descriptions or documentation. Write two sentences instead.

# Workflow

You *MUST* read `HACKING.md` before doing any changes in this repo. You *MUST* follow the guidelines in `HACKING.md`, consider it a part of this document.

Build `check_style` and `check_tidy` targets to check style. You *MUST* always check style after your changes.

Build `Run_UnitTest` and `Run_GameTest_Headless_Parallel` targets to test your changes. You *MUST* always run tests after your changes. If you can't find the game data - ask the user to help you locate it, *NEVER* silently skip game tests.

*NEVER* claim anything about game data or runtime behaviour that you haven't checked. OpenEnroth targets MM6, MM7 and MM8, so a claim about the data means all three were scanned. Go the extra mile when scanning data, and say what you checked - "no shipped record does this" means MM6, MM7 and MM8 scripts were all read, and if one of them wasn't, say which.

Before every commit, do a deletion pass over the comments in the diff, as a separate step. For each added comment ask whether it describes the code or defends the change, and delete the ones that defend it. Then check what's left against the Comments section. You *MUST* do this, the review rounds exist for design questions, not for comment cleanup you could have caught yourself.

*NEVER* amend a commit or rewrite pushed history unless explicitly asked to. Fixes go on top as new commits with their own messages, and squashing is the human's call. The one exception is keeping up with master. When a PR branch falls behind or conflicts, rebase it onto master and push with `--force-with-lease`. *NEVER* merge master into it.

Push your work to the branch it belongs on, an open PR under review included. For a follow-up commit on an open PR, build and run the tests it touches, then push. The style checks, the full test suite and CI run after the push. *NEVER* sit in a loop waiting for them, the human wants the conversation back. Launch them detached and report the results once they're in. A failure found later goes on top as its own commit.

The commit message is where the reasoning lives. Say what was wrong and why this is the fix, in a paragraph. Every "why" about the diff that you were tempted to put in a comment goes here instead. A "why" about the game or the platform stays in the source, see Comments.

Keep pull request descriptions short - a few sentences on what was done and how, and that's it. Two or three paragraphs at most, humans don't want to read an essay. The evidence you gathered along the way belongs in a comment on the PR if it belongs anywhere.

Put the `🤖 Human Needed` label on every pull request you open, as part of opening it. The label tells the humans that an agent's PR is waiting for its first look. Taking it off is the human's call, and once it's off it stays off, *NEVER* put it back. It's the only label you may ever add, and you *NEVER* remove a label, this one included.

# Comments

The default is no comment. Write one only for what the code can't say: a non-obvious runtime or domain fact, a hidden dependency, or a value that looks wrong but isn't. Most comments in this codebase are one line, and usually trail the code they're about, match that.

A comment describes the code. It *NEVER* defends the change. A sentence about the game, the data, the platform or a library describes. A sentence about this diff defends, and so does one about what you didn't do. Delete it, it's commit message material. "Because" is fine when a fact about the world follows it.

Every sentence in a comment must say something the code, the names and the previous sentence don't. A comment that repeats the name next to it is not needed. If a call site needs a comment to be readable, fix the code instead. An enum parameter reads at the call site. A bool doesn't.

Explanations live where the logic lives. A comment about a statement is a trailing comment on that statement. A comment about a return sits at the return. A comment about a call sits at the call. The doxygen block above a function is for callers only, what it does, what goes in and what comes out, never a walkthrough of the body.

A function, class, struct, enum or table whose name doesn't say it all gets a doxygen block, with a `@param` tag for each parameter and a `@return` tag if there's a result. The text after a tag starts at column 41. A name that says it all gets nothing, `isOpen()` needs no block. A test, and the helpers next to it, get no block, a `//` line at the top of the body does the job. A comment above a declaration is a doxygen block or nothing, a `//` line you'd write there either grows into the block or moves inside the body. Existing `//----- (0x...)` offset markers stay, see `HACKING.md`. A `TODO(name)` above a declaration is fine, it says what's left to do and nothing else.

An invariant is an assert, not a comment.

Production code never narrates past bugs. A test does the opposite, its first line names the bug it guards against. "Gold piles were generated with 0 gold" is the shape. When a comment does mention a past bug, say it was a bug. "We used to keep the old buffer" reads like a choice. "This used to be a heap buffer overflow" doesn't.

# Game tests

You write game tests in code. The trace recording workflow in `HACKING.md` is for humans. A test starts with `game.startNewGame()` and builds everything it needs from there. *NEVER* add a trace or a savegame to `test/Data`. An existing trace test that desyncs after your change gets retraced, as `HACKING.md` describes. If a retrace can't fix it, or the test no longer guards its bug, rewrite it in code and delete its trace and save.

A test reproduces what a player would do and checks what a player would see:
* Reproduce the scenario from the issue, as the issue describes it. A test of something nearby that happens to break too is not a test for this issue. A second scenario is welcome as its own test, but the original one comes first.
* Act through input. Reach a house by teleporting in front of its door and pressing Space, not with `enterHouse()`. Select a character with a digit key, not with `setActiveCharacterIndex()`. Aim with the `pointMouseAt*` helpers and `hoverGuiButton()`, then click with `pressAndReleaseButton(BUTTON_LEFT)`.
* Let time pass with `game.tick()`, *NEVER* by writing to the party's playing time. A frame is 100 ms and game time runs 30 times faster, so `tick(100)` is five game minutes. Use the fewest ticks that work, a single `tick()` is usually enough for an input to land. Don't comment on how many ticks you picked.
* Check the result where the bug shows. A test that only looks a value up in a table proves nothing, go to the shop and check the greeting that played.
* A bug a player can hit gets a game test against the real game data, not a unit test with made-up data. If a fix has no symptom a player could see, ask before writing a test for it.

Observe through tapes. Create them before the action, from `tapes`, `actorTapes` and `charTapes`, and compare whole tapes with `EXPECT_EQ(tape, tape(...))`, `EXPECT_CONTAINS` or `delta()`. Don't save a value into a local before the action and compare after, and don't search engine arrays after the fact. A missing tape goes into `CommonTapeRecorder` or a `tapes.custom()` lambda. Nothing is taped until you call `test.startTaping()`. Call it right after `game.startNewGame()`, or after the last map load when a tape reads level data, because loading screen frames get taped too. Don't comment on where it sits. `test.stopTaping()` is only needed when later frames would pollute a tape.

Tape everything the scenario touches, not only the value the fix changes. The other tapes are what tell the right fix from a wrong one. When a fix lets paralyzed monsters take damage, a stoned titan next to the paralyzed one shows that it didn't make every monster take damage, and the titan taking the direct hit shows that the spell landed at all. When a script swaps two quest bits, tape both, because one bit alone would also pass on a script that ran halfway.

Pin the behavior exactly. Prefer `EXPECT_EQ` over a range when the value is deterministic, "respawns at 1 HP" is checked as 1 HP, not as "at most 2". Don't recompute game formulas in the test though. A damage roll is checked as "damage was dealt", unless the damage range is what the test is about, then check the range. Use `ASSERT_*` when the checks after it would be meaningless on failure.

Keep the test minimal. Don't set up state the bug doesn't need, like a skill nobody uses. Spawn monsters with `SPAWN_DUMMY` when they only need to stand there and take hits. A dummy is level 1 with no resistances, so it never wins a resistance roll and every spell and weapon effect lands on it. When the test is about resistance rolls, keep the monster's level and resistances and spawn it with `SPAWN_STATIONARY` alone. A spawned monster is hostile. Add `SPAWN_FRIENDLY` when the test needs a friendly one, an NPC to talk to or a peasant to steal from.

When the same check has to hold for a case that should work and a case that shouldn't, write a two-iteration loop over both in one test. Two separate concerns, or two code paths, are two tests with letter suffixes, like `Issue1262a` and `Issue1262b`.

A test is named after the issue it guards, `Issues.Issue1342`, or after the PR when there's no issue, `Prs.Pr2599`. It goes into the `GameTests_*.cpp` file whose range holds its number, sorted by number. Static helpers go at the top of the file, after the includes. The first line names the bug as a symptom, per the Comments section. For a test that guards behavior that was never broken, say how the game should work instead. For a bug that came and went inside your own unmerged PR, describe the behavior, there was no bug on master to name. No engine literals in comments, "reaches 307" means nothing to a reader.

Before you push, revert the fix and watch the test fail, then restore it. Asserts are enabled in release builds too, so a test that only fails through an assert is still a good test. Say in the PR description that you did this.

When you need to know how the game behaves before claiming it, write a throwaway game test, run it, and delete it. Its code goes into the issue if you file one.
