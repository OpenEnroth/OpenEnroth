# OpenEnroth

[![Build](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/build_release.yml/badge.svg)](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/build_release.yml)
[![Lint](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/lint.yml/badge.svg)](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/lint.yml)

OpenEnroth is an open-source reimplementation of the Might & Magic VI-VIII game engine, allowing you to play the
original games on modern platforms. It uses the original game data, so you will need a copy of the game to play.

This repository is a fork of [OpenEnroth](https://github.com/OpenEnroth/OpenEnroth) that adds Might and Magic VIII
support: MM7 and MM8 are playable here. Upstream's progress on MM6 and MM8 is tracked in its
[milestones](https://github.com/OpenEnroth/OpenEnroth/milestones).

Supported platforms: **Windows**, **Linux**, **macOS**, and **Android** (experimental).

![screenshot_main](https://user-images.githubusercontent.com/24377109/79051217-491a7800-7c2f-11ea-85c7-f9120b7d79dd.png)

# Download

This fork has no prebuilt binaries. To play MM8, build it from source as described under
"Might and Magic VIII (this fork)" below.

Upstream OpenEnroth publishes nightly builds on its [releases page](https://github.com/OpenEnroth/OpenEnroth/releases).
They play MM7 but do not contain this fork's MM8 support. The installation steps below are for those upstream builds.

# Discord

Join our Discord to discuss, track progress, or get involved in development.

[![Discord channel invite](https://img.shields.io/badge/chat-on%20discord-green.svg)](https://discord.gg/jRCyPtq) 


# Getting Started

You will need two things: the OpenEnroth binary and the original Might and Magic VII game data.

## Getting the game data

You can buy Might and Magic VII from [GOG.com](https://www.gog.com/en/game/might_and_magic_7_for_blood_and_honor).
Copies from other sources (e.g. an original retail disc) should also work.

At the very least, OpenEnroth requires the `ANIMS`, `DATA`, `MUSIC` and `SOUNDS` directories from the game data.

### Non-GOG versions

Install or extract your copy of the game as normal.

### GOG Version

#### Windows

On Windows, run the GOG installer to extract the game data to your computer.

#### Linux and macOS

The installer cannot be run directly on Linux and macOS, but you can use `innoextract` to extract its contents.

* On macOS, install [Homebrew](https://brew.sh/) if you don't have it already and run `brew install innoextract`
* On Linux, the exact command depends on your distribution:
  * Ubuntu or Debian-based (including Linux Mint, Pop!OS, etc): `sudo apt install -y innoextract`
  * Fedora or other RedHat-based (*except* "immutable" distributions like Bazzite): `sudo dnf install -y innoextract`
  * For other systems, [Repology](https://repology.org/project/innoextract/versions) lists the package name for most distributions.

Once you have innoextract, run:

`innoextract -e -d <new game data directory> <path to GOG installer .exe>`

### Game Assets Path Override

You can set an environment variable called `OPENENROTH_MM7_PATH` to point to the location of the game data.
If set, OpenEnroth will look for game assets only in that location. You may want to add this to your shell
profile (e.g. `~/.profile` on Linux or `~/.zshrc` on Mac):

```
export OPENENROTH_MM7_PATH="<path-to-mm7-game-assets>"
```

## Installing OpenEnroth

### Windows

1. Download `OpenEnroth_nightly_Windows_x86_64_RelWithDebInfo.zip` (`x86` for 32-bit Windows) from the
   [releases page](https://github.com/OpenEnroth/OpenEnroth/releases).
2. Unzip `OpenEnroth.exe` and `OpenEnroth.pdb` into the directory containing the game data.
3. Run `OpenEnroth.exe`.

### macOS

1. Move the game data to `~/Library/Application Support/OpenEnroth`, creating this directory if needed.
2. Download `OpenEnroth_nightly_macOS_arm64_RelWithDebInfo.dmg` (`x86_64` for Intel Macs) from the
   [releases page](https://github.com/OpenEnroth/OpenEnroth/releases), open it and drag `OpenEnroth.app` into
   `Applications`.
3. Run `xattr -rc /Applications/OpenEnroth.app`. This clears the quarantine flag — macOS will refuse to
   launch unsigned binaries without this step.
4. Run `OpenEnroth.app`.

### Linux

OpenEnroth ships for Linux as a Flatpak package.

1. Check for Flatpak support:
   * Run `flatpak --version`. If you get a version number, you're ready; otherwise visit
     [https://flatpak.org/setup/](https://flatpak.org/setup/) for setup instructions.
2. Install OpenEnroth:
   * Download the `OpenEnroth_nightly_Linux_x86_64_RelWithDebInfo.flatpak` package from the [releases page](https://github.com/OpenEnroth/OpenEnroth/releases).
   * Run `flatpak install --user /path/to/OpenEnroth_nightly_Linux_x86_64_RelWithDebInfo.flatpak`.
   * Create `~/.var/app/io.github.openenroth.openenroth/data/mm7/data/`
   * Move the game data (at least `ANIMS`, `DATA`, `MUSIC` and `SOUNDS`) into this new directory.
3. Run OpenEnroth from your application menu or using `flatpak run io.github.openenroth.openenroth`.

### Android (Experimental)

Android is not actively tested by the dev team — expect issues and be prepared to troubleshoot.

1. Download `OpenEnroth_nightly_Android_universal_RelWithDebInfo.apk` from the [releases page](https://github.com/OpenEnroth/OpenEnroth/releases).
2. Install the APK on your device (you will need to allow installation from unknown sources in your device settings).
3. Copy the game data (`ANIMS`, `DATA`, `MUSIC` and `SOUNDS`) to
   `/sdcard/Android/data/io.github.openenroth.openenroth/files/` on your device.


# Might and Magic VIII (this fork)

This fork adds Might and Magic VIII support. It ships no game content, so you need your own copy of MM8. There are no prebuilt
binaries, you build it yourself.

1. Get the MM8 game data. Extract the installer as described under "GOG Version" above, with
   `innoextract -e -d <mm8 data directory> <path to installer .exe>`. The directory must contain `Anims`,
   `Data`, `Music` and `Sounds`, and OpenEnroth recognizes it as MM8 by `Anims/magicdod.vid`.
2. Build, using the commands from [HACKING.md](HACKING.md) (CMake 3.27+, a C++ compiler new enough for it, and the
   submodules):

   ```
   git clone --recurse-submodules --shallow-submodules https://github.com/JahJajaka/OpenEnroth OpenEnroth
   cd OpenEnroth
   cmake -B build -S . -DCMAKE_BUILD_TYPE=RelWithDebInfo
   cmake --build build
   ```

3. Run it, passing the data directory. On Linux:

   ```
   ./build/src/Bin/OpenEnroth/OpenEnroth --data-path "<mm8 data directory>"
   ```

   On macOS the binary is inside the app bundle:

   ```
   ./build/src/Bin/OpenEnroth/OpenEnroth.app/Contents/MacOS/OpenEnroth --data-path "<mm8 data directory>"
   ```

4. Optionally run the unit tests, which need no game data. The game tests in this tree are upstream's own and
   need MM7 data.

   ```
   cmake --build build --target Run_UnitTest
   ```

# Development

See [HACKING.md](HACKING.md) for build instructions, code style guidelines, and contribution guidelines.


# License

OpenEnroth is licensed under the LGPL-3.0, see [LICENSE](LICENSE). Our changes to this fork are also offered under the MIT License, and upstream code stays under the LGPL-3.0.

# Screenshots

![screenshot_1](https://user-images.githubusercontent.com/24377109/79051879-f04cde80-7c32-11ea-939d-1dcc97b46f5d.png)

![screenshot_2](https://user-images.githubusercontent.com/24377109/79051881-f17e0b80-7c32-11ea-82cd-5e4993a1c071.png)

![screenshot_3](https://user-images.githubusercontent.com/24377109/79051882-f3e06580-7c32-11ea-974f-414f68394190.png)

![screenshot_4](https://user-images.githubusercontent.com/24377109/79051883-f5119280-7c32-11ea-801c-1595709d8060.png)
