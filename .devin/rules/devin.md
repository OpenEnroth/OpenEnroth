---
trigger: always_on
---

You *MUST* read `HACKING.md` before doing any changes in this repo. You *MUST* follow the guidelines in `HACKING.md`, consider it a part of this document.

## Build configuration

This project uses CMake with Visual Studio 2022+ on Windows, GCC 15+ on Linux, or AppleClang 16+ on macOS.

- By default, prebuilt dependencies are downloaded automatically (`OE_USE_PREBUILT_DEPENDENCIES=ON`).
- If prebuilt download fails or system dependencies are used, pass `-DOE_USE_PREBUILT_DEPENDENCIES=OFF` and ensure system packages (zlib, ffmpeg, SDL3, OpenAL, etc.) are installed.
- For style-only checks without real dependencies, use `-DOE_USE_DUMMY_DEPENDENCIES=ON`. This allows building `check_style` but *not* unit or game tests.
- The build directory is typically `out/build/<config-name>/` (per `CMakeSettings.json`).

## Style checks

Build `check_style` target to check style. You *MUST* always check style after your changes.

```
cmake --build out/build/x64-Debug --target check_style
```

## Unit tests

Build and run the `Run_UnitTest` target (which depends on `OpenEnroth_UnitTest`). You *MUST* always run unit tests after your changes.

```
cmake --build out/build/x64-Debug --target Run_UnitTest
```

Unit tests require real dependencies (not dummy deps).

## Game tests

Build and run the `Run_GameTest_Headless_Parallel` target. You *MUST* always run game tests after your changes. If you can't find the game data - ask the user to help you locate it, *NEVER* silently skip game tests.

Game tests require:
- The `OPENENROTH_MM7_PATH` environment variable pointing to MM7 game data (at minimum `ANIMS`, `DATA`, `MUSIC`, `SOUNDS` directories).
- Real dependencies (not dummy deps).

```
$env:OPENENROTH_MM7_PATH = "<path-to-mm7-game-data>"
cmake --build out/build/x64-Debug --target Run_GameTest_Headless_Parallel
```

On Linux/macOS:
```
export OPENENROTH_MM7_PATH="<path-to-mm7-game-data>"
cmake --build build --target Run_GameTest_Headless_Parallel
```

If game tests fail with `Random state desynchronized`, see `HACKING.md` section on retracing traces.

## Before submitting changes

1. Style check passes (`check_style` target).
2. Unit tests pass (`Run_UnitTest` target).
3. Game tests pass (`Run_GameTest_Headless_Parallel` target).
4. If you changed game logic and traces desynchronized, retrace and commit updated traces.
