# Might and Magic Trilogy

[![Windows](https://github.com/OpenEnroth/OpenEnroth/workflows/Windows/badge.svg)](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/windows.yml) 
[![Linux](https://github.com/OpenEnroth/OpenEnroth/workflows/Linux/badge.svg)](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/linux.yml) 
[![MacOS](https://github.com/OpenEnroth/OpenEnroth/workflows/MacOS/badge.svg)](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/macos.yml) 
[![Doxygen](https://github.com/OpenEnroth/OpenEnroth/workflows/Doxygen/badge.svg)](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/doxygen.yml) 
[![Style Checker](https://github.com/OpenEnroth/OpenEnroth/workflows/Style/badge.svg)](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/style.yml)

We are creating an extensible engine & modding environment that would make it possible to play original Might & Magic VI-VIII
games on modern platforms with improved graphics and quality-of-life features expected of modern titles, and make modding and
installing & playing the mods a pleasurable experience.

Currently only MM7 is playable. You can check out the [milestones](https://github.com/OpenEnroth/OpenEnroth/milestones) to
see where we're at.

![screenshot_main](https://user-images.githubusercontent.com/24377109/79051217-491a7800-7c2f-11ea-85c7-f9120b7d79dd.png)

# Download

To download the code without having to compile it we have our releases at
[https://github.com/OpenEnroth/OpenEnroth/releases](https://github.com/OpenEnroth/OpenEnroth/releases) 

Currently there are only the nightly builds which may have bugs.

# Discord

Join our discord channel to discuss, track progress or get involved in the development of this project.

[![Discord channel invite](https://img.shields.io/badge/chat-on%20discord-green.svg)](https://discord.gg/jRCyPtq) 


# Getting Started

Regardless of what system you want to play OpenEnroth on, you will need a copy of OpenEnroth and the Might and Magic VII
game data. Where and how you install these on your computer depends on your operating system.

## Getting the game data

You can buy Might and Magic VII from [gog.com](https://www.gog.com/en/game/might_and_magic_7_for_blood_and_honor) and
download the installer from there; if you have a copy from another source (eg. an original retail disc) this should
also work with OpenEnroth.

At the very least, OpenEnroth requires the `ANIMS`, `DATA`, `MUSIC` and `SOUNDS` directories from the game data.

### Non-GOG versions

Install or extract your copy of the game as normal.

### GOG Version

#### Windows

On Windows, you can simply run the installer to extract the game data to your computer.

#### Linux and macOS

The installer cannot be run directly on Linux and macOS but you can use `innoextract` to extract its contents.

* On macOS, install [Homebrew](https://brew.sh/) if you don't have it already (run `brew --version` in a Terminal
  window to see if you do) and install `innoextract` using `brew install innoextract`
* On Linux, the exact command depends on your distribution.
  * For Ubuntu or Debian-based systems (including eg. Linux Mint, Pop!OS, etc): `sudo apt install -y innoextract`
  * For Fedora or other RedHat-based systems (*except* for "immutable" distributions like Bazzite):
    `sudo dnf install -y innoextract`
  * For other systems, please check the distribution's user guide. [Repology](https://repology.org/project/innoextract/versions)
    has an extensive list of Linux distributions and the name of the package for innoextract on each.

Once you have innoextract, create a new directory for the game data, then open a terminal window and run:

`innoextract -e -d <new game data directory> <path to GOG installer .exe>`

This will extract the game data to the new directory.

### Game Assets Path Override

You can set an environment variable called `OPENENROTH_MM7_PATH` to point to the location of the game data.
If this variable is set, OpenEnroth will look for game assets only in the location it's pointing to. You
might also want to add the following line to your shell profile (e.g. `~/.profile` on Linux or `~/.zshrc` on Mac):

```
export OPENENROTH_MM7_PATH="<path-to-mm7-game-assets>"
```

## Installing OpenEnroth

### Windows

1. Download one of the prebuilt [releases](https://github.com/OpenEnroth/OpenEnroth/releases) and unzip the files.
2. Copy `OpenEnroth.exe` and `OpenEnroth.pdb` to the directory containing the game data.
3. Run `OpenEnroth.exe`.

### macOS

1. Move the game data to `~/Library/Application Support/OpenEnroth`, creating this directory if needed.
2. Download one of the prebuilt [releases](https://github.com/OpenEnroth/OpenEnroth/releases) and unzip
   the files.
3. Run `xattr -rc <extracted-path>/dist/OpenEnroth.app`. This is needed because OpenEnroth binaries are
   unsigned, without this step the app bundle won't start.
4. Run `OpenEnroth.app`.

### Linux (Flatpak)

The Flatpak package is the easiest choice if you aren't using Ubuntu 24.04, or you cannot install system
packages on your computer for any reason (as long as Flatpak is available), e.g. because you're using
an "atomic"/"immutable" Linux distribution such as Bazzite or SteamOS.

1. Check for Flatpak support:
   * Run `flatpak --version`.
   * If you receive a message listing a version number, you're ready to go; otherwise please visit
    [https://flatpak.org/setup/](https://flatpak.org/setup/) and follow instructions there to set up.
2. Install OpenEnroth:
   * Download the `io.github.openenroth.openenroth_*.flatpak` package from one of the
     prebuilt [releases](https://github.com/OpenEnroth/OpenEnroth/releases).
   * Run `flatpak install --user /path/to/io.github.openenroth.openenroth_*.flatpak` to install the OpenEnroth
     package.
   * Create `~/.var/app/io.github.openenroth/openenroth/data/mm7/data/`
   * Move the game data (at least the `ANIMS`, `DATA`, `MUSIC` and `SOUNDS` directories) into this new directory
3. Run OpenEnroth from your desktop's application menu or using `flatpak run io.github.openenroth.openenroth`

### Linux (Loose executable)

The loose executable bundle is the easier option if:
* You want easy access to the OpenEnroth executable for development or debugging
* You absolutely need specific control over the installation location for OpenEnroth and the game data
* You can install system packages (specifically, `libdwarf`, `libelf` and `libgl1`)
* You're using Ubuntu 24.04 - other distributions may not have the required libraries available
  or not in compatible versions

1. Install the required libraries (`libdwarf`, `libelf` and `libgl1`):
   * `sudo apt-get install libdwarf1 libelf++* libgl1` (Ubuntu 24.04)
   * For other distributions, check your distribution's user guides and package management tools to find
     the correct package names for these libraries.
2. Install OpenEnroth:
   * Download one of the prebuilt [releases](https://github.com/OpenEnroth/OpenEnroth/releases) and unzip
     the files
   * Copy the game data (at least the `ANIMS`, `DATA`, `MUSIC` and `SOUNDS` directories) to the directory
     you unzipped the release to, next to the `OpenEnroth` executable
4. Run the `OpenEnroth` executable (you may need to run `chmod a+x OpenEnroth` first) from a terminal
   or by double-clicking it in a file browser.


# Development

See the [HACKING](HACKING.md) document for information on how to compile or if you intend to contribute.

See the code [DOCUMENTATION](https://openenroth.github.io/OpenEnroth/index.html).

# Screenshots

![screenshot_1](https://user-images.githubusercontent.com/24377109/79051879-f04cde80-7c32-11ea-939d-1dcc97b46f5d.png)

![screenshot_2](https://user-images.githubusercontent.com/24377109/79051881-f17e0b80-7c32-11ea-82cd-5e4993a1c071.png)

![screenshot_3](https://user-images.githubusercontent.com/24377109/79051882-f3e06580-7c32-11ea-974f-414f68394190.png)

![screenshot_4](https://user-images.githubusercontent.com/24377109/79051883-f5119280-7c32-11ea-801c-1595709d8060.png)
