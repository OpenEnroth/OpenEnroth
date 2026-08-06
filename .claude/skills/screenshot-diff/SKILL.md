---
name: screenshot-diff
description: Pixel-compare in-game rendering between two revisions (e.g. working tree vs HEAD) by driving both builds to the same screens under Xvfb and diffing full-frame screenshots. Use when a rendering change must be proven pixel-identical, or when hunting a visual regression that should be verified by pixels rather than by eye.
---

Compare what the game actually draws between two revisions. The output is a per-screen pixel diff count
plus side-by-side images with differences highlighted in red — converge to 0 differing pixels, or use the
highlighted regions to localize the regression.

**This workflow is written for the Linux devcontainer and may not work elsewhere.** It relies on `Xvfb` +
mesa's llvmpipe from the devcontainer image (see `.devcontainer/Dockerfile`), the `/workspace` layout, and
`/usr/bin/gcc` paths. On a macOS host or any machine with a real GPU it needs adaptation: skip Xvfb and run
windowed instead, and note that pixel-identity is only meaningful when BOTH sides render on the same
rasterizer — comparing screenshots taken on different GPUs/drivers produces spurious diffs.

## Prerequisites

- `Xvfb` and mesa's llvmpipe (both installed in the devcontainer). Headless mode uses `NullRenderer` and
  screenshots come back black — the comparison MUST run non-headless under Xvfb.
- Game data at `$OPENENROTH_MM7_PATH` (usually `/workspace/OpenEnroth_GameData/mm7`).
- Pillow, for `compare_shots.py` (`python3-pil`, also in the devcontainer).

## Setup: two builds

The "new" side is the working tree, built in its usual build dir. For the "old" side, make a worktree:

```sh
git worktree add --detach /tmp/oe_old <OLD_REF>
cmake -B /tmp/oe_old/build -S /tmp/oe_old -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=/usr/bin/gcc -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DCMAKE_ASM_COMPILER=/usr/bin/gcc
```

**Do NOT run `git submodule update` inside the worktree** — it hijacks the main repo's submodule gitdirs
(`core.worktree` gets repointed) and breaks `git status` in the main checkout. Copy submodule sources from
the main tree instead, and drop their `.git` pointer files so they become plain source dirs:

```sh
cp -a /workspace/thirdparty/. /tmp/oe_old/thirdparty/
find /tmp/oe_old/thirdparty -maxdepth 2 -name .git -type f -delete
```

If a submodule differs between the two revisions, fetch that one separately from its own clone.

## Instrumentation

Append the test from `screenshot_test.cpp.inc` (in this skill's directory) to
`test/Bin/GameTest/GameTests_0000.cpp` in BOTH trees, with the includes listed at its top. Adjust the
driven screens to the change under test — the template does the character window's four tabs. It writes
one PNG per screen into `$OE_SHOT_DIR`. This is temporary instrumentation: remove it from the working
tree before committing (`git checkout -- test/Bin/GameTest/GameTests_0000.cpp` once done).

Build `OpenEnroth_GameTest` in both build dirs.

## Run and compare

```sh
Xvfb :99 -screen 0 1024x768x24 &
export DISPLAY=:99 OPENENROTH_MM7_PATH=/workspace/OpenEnroth_GameData/mm7

mkdir -p /tmp/shots/{old,new,diff}
OE_SHOT_DIR=/tmp/shots/old /tmp/oe_old/build/test/Bin/GameTest/OpenEnroth_GameTest \
    --test-path /tmp --gtest_filter=Screenshots.CharacterScreen
OE_SHOT_DIR=/tmp/shots/new <NEW_BUILD_DIR>/test/Bin/GameTest/OpenEnroth_GameTest \
    --test-path /tmp --gtest_filter=Screenshots.CharacterScreen

python3 .claude/skills/screenshot-diff/compare_shots.py /tmp/shots/old /tmp/shots/new /tmp/shots/diff
```

Notes:
- No `--headless`. `--test-path` is mandatory for the binary but unused by this test — any existing dir works.
- `compare_shots.py` exits 0 only on pixel-identical screenshots; otherwise it prints per-file diff counts
  and writes `diff_<name>.png` (old | new | old-with-diffs-in-red) for inspection. Read the marked pane to
  see WHERE the diff is before theorizing about why.

## Loop

Fix → rebuild the new side only → rerun the new side → compare. The old side's screenshots are stable;
never rebuild or rerun it unless the old ref itself changes.

## Cleanup

- Remove the instrumentation from the working tree; rebuild `OpenEnroth_GameTest` so the checked binary
  matches the clean sources.
- `git worktree remove /tmp/oe_old --force`
- Kill the Xvfb if no longer needed: `pkill Xvfb`.
