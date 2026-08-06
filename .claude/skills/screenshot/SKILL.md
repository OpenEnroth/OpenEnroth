---
name: screenshot
description: Capture what the game actually draws, by driving it to a chosen screen under Xvfb and saving full-frame PNGs. Use when you need to see how a feature renders, or to pixel-compare two revisions when a rendering change must be proven identical or a visual regression localized.
---

Two uses:

- **Look at a screen.** Drive the game to whatever the change affects and save PNGs to inspect.
- **Compare two revisions.** Do the same in both builds and diff the frames, converging to 0 differing
  pixels, or using the highlighted regions to localize a regression.

**This workflow is written for the Linux devcontainer and may not work elsewhere.** It relies on `Xvfb` +
mesa's llvmpipe from the devcontainer image (see `.devcontainer/Dockerfile`) and the `/workspace` layout.
On a macOS host or any machine with a real GPU it needs adaptation: skip Xvfb and run windowed instead.

## Prerequisites

- `Xvfb` and mesa's llvmpipe (both installed in the devcontainer). Headless mode uses `NullRenderer` and
  screenshots come back black — this MUST run non-headless under Xvfb.
- Game data at `$OPENENROTH_MM7_PATH` (usually `/workspace/OpenEnroth_GameData/mm7`).
- Pillow, for `compare_shots.py` (`python3-pil`, also in the devcontainer).

## Taking screenshots

Append the test from `screenshot_test.cpp.inc` (in this skill's directory) to
`test/Bin/GameTest/GameTests_0000.cpp`, with the includes listed at its top, and adjust the driven screens
to whatever you want to see — the template does the character window's four tabs. It writes one PNG per
screen into `$OE_SHOT_DIR`. This is temporary instrumentation: remove it before committing
(`git checkout -- test/Bin/GameTest/GameTests_0000.cpp`).

Build `OpenEnroth_GameTest`, then:

```sh
Xvfb :99 -screen 0 1024x768x24 &
export DISPLAY=:99 OPENENROTH_MM7_PATH=/workspace/OpenEnroth_GameData/mm7

mkdir -p /tmp/shots
OE_SHOT_DIR=/tmp/shots <BUILD_DIR>/test/Bin/GameTest/OpenEnroth_GameTest \
    --test-path /tmp --gtest_filter=Screenshots.CharacterScreen
```

No `--headless`. `--test-path` is mandatory for the binary but unused by this test — any existing dir works.

Read the PNGs directly to see what the game drew.

## Comparing two revisions

Pixel-identity is only meaningful when BOTH sides render on the same rasterizer — screenshots taken on
different GPUs or drivers produce spurious diffs.

The "new" side is the working tree, built in its usual build dir. For the "old" side, take a clean clone —
not a `git worktree`, because `git submodule update` inside a worktree repoints the main repo's submodule
gitdirs and breaks `git status` in your main checkout:

```sh
git clone /workspace /tmp/oe_old
cd /tmp/oe_old
git checkout <OLD_REF>
git submodule update --init --recursive   # superproject came from the local clone, submodules from network
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release
```

Add the same instrumentation to BOTH trees and build `OpenEnroth_GameTest` in both build dirs. Then shoot
both into separate directories and diff:

```sh
mkdir -p /tmp/shots/{old,new,diff}
OE_SHOT_DIR=/tmp/shots/old /tmp/oe_old/build/test/Bin/GameTest/OpenEnroth_GameTest \
    --test-path /tmp --gtest_filter=Screenshots.CharacterScreen
OE_SHOT_DIR=/tmp/shots/new <NEW_BUILD_DIR>/test/Bin/GameTest/OpenEnroth_GameTest \
    --test-path /tmp --gtest_filter=Screenshots.CharacterScreen

python3 .claude/skills/screenshot/compare_shots.py /tmp/shots/old /tmp/shots/new /tmp/shots/diff
```

`compare_shots.py` exits 0 only on pixel-identical screenshots; otherwise it prints per-file diff counts and
writes `diff_<name>.png` (old | new | old-with-diffs-in-red) for inspection. Read the marked pane to see
WHERE the diff is before theorizing about why.

To iterate: fix → rebuild the new side only → rerun the new side → compare. The old side's screenshots are
stable; never rebuild or rerun it unless the old ref itself changes.

## Cleanup

- Remove the instrumentation; rebuild `OpenEnroth_GameTest` so the checked binary matches clean sources.
- `rm -rf /tmp/oe_old`
- Kill the Xvfb if no longer needed: `pkill Xvfb`.
