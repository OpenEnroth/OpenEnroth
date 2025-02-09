# Might and Magic Trilogy

[![Windows](https://github.com/OpenEnroth/OpenEnroth/workflows/Windows/badge.svg)](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/windows.yml) [![Linux](https://github.com/OpenEnroth/OpenEnroth/workflows/Linux/badge.svg)](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/linux.yml) [![MacOS](https://github.com/OpenEnroth/OpenEnroth/workflows/MacOS/badge.svg)](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/macos.yml) [![Doxygen](https://github.com/OpenEnroth/OpenEnroth/workflows/Doxygen/badge.svg)](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/doxygen.yml) [![Style Checker](https://github.com/OpenEnroth/OpenEnroth/workflows/Style/badge.svg)](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/style.yml)

We are creating an extensible engine & modding environment that would make it possible to play original Might&Magic VI-VIII games on modern platforms with improved graphics and quality-of-life features expected of modern titles, and make modding and installing & playing the mods a pleasurable experience.

Currently only MM7 is playable. You can check out the [milestones](https://github.com/OpenEnroth/OpenEnroth/milestones) to see where we're at.

![screenshot_main](https://user-images.githubusercontent.com/24377109/79051217-491a7800-7c2f-11ea-85c7-f9120b7d79dd.png)

Download
-------

To download the code without having to compile it we have our releases at [https://github.com/OpenEnroth/OpenEnroth/releases](https://github.com/OpenEnroth/OpenEnroth/releases) 

Currently there are only the nightly builds which may have bugs.

Discord
-------

Join our discord channel to discuss, track progress or get involved in the development of this project.

[![Discord channel invite](https://img.shields.io/badge/chat-on%20discord-green.svg)](https://discord.gg/jRCyPtq) 


Getting Started on Windows
--------------------------

1. You will need a GoG or any other version of Might and Magic VII. Run the installer as usual.

2. Download one of the prebuilt [releases](https://github.com/OpenEnroth/OpenEnroth/releases) and unzip the files.

3. From the `dist` folder inside the zip file copy `OpenEnroth.exe` and `OpenEnroth.pdb` to the folder where the game is installed.

4. Run `OpenEnroth.exe`.


Getting Started on Ubuntu Linux
-------------------------------

1. You will need a GoG or any other version of Might and Magic VII. If you have an offline GoG installer, then follow these steps:
   * Install `innoextract` with `sudo apt-get install innoextract`.
   * Run `innoextract -e -d <target-folder> <mm7-gog-folder>/setup_mm_7.exe`, where `<target-folder>` is the folder where you want to have game data extracted from the mm7 installer.
   * Check the files in `<target-folder>`, it should now contain the `app` subfolder. This is where game assets were extracted into.

2. Install OpenEnroth dependencies with `sudo apt-get install libsdl2-dev libdwarf-dev libelf-dev`.

3. Download one of the prebuilt [releases](https://github.com/OpenEnroth/OpenEnroth/releases) and unzip the files.

4. From the `dist` folder inside the zip file copy the `OpenEnroth` binary to `<target-folder>/app`.

5. Run `OpenEnroth` binary.


Getting Started on MacOS
------------------------

1. You will need a GoG or any other version of Might and Magic VII. If you have an offline GoG installer, then follow these steps:
   * Install `innoextract` with `brew install innoextract`.
   * Run `innoextract -e -d <target-folder> <mm7-gog-folder>/setup_mm_7.exe`, where `<target-folder>` is the folder where you want to have game data extracted from the mm7 installer.
   * Check the files in `<target-folder>`, it should now contain the `app` subfolder. This is where game assets were extracted into.

2. Move extracted game assets to `~/Library/Application Support/OpenEnroth`. This is where `OpenEnroth` will look for game data.

3. Download one of the prebuilt [releases](https://github.com/OpenEnroth/OpenEnroth/releases) and unzip the files.
   
4. Run `xattr -rc <extracted-path>/dist/OpenEnroth.app`. This is needed because OpenEnroth binaries are unsigned, without this step the app bundle won't start.

5. Start `OpenEnroth.app`.


Game Assets Path Override
-------------------------

You can set `OPENENROTH_MM7_PATH` env variable to point to the location of the game assets. If this variable is set, OpenEnroth will look for game assets only in the location it's pointing to. You might also want to add the following line to your bash profile (e.g. `~/.profile` on Ubuntu):

```
export OPENENROTH_MM7_PATH="<path-to-mm7-game-assets>"
```


Development
-----------

See the [HACKING](HACKING.md) document for information on how to compile or if you intend to contribute.

See the code [DOCUMENTATION](https://openenroth.github.io/OpenEnroth/index.html).

Screenshots
-----------

![screenshot_1](https://user-images.githubusercontent.com/24377109/79051879-f04cde80-7c32-11ea-939d-1dcc97b46f5d.png)

![screenshot_2](https://user-images.githubusercontent.com/24377109/79051881-f17e0b80-7c32-11ea-82cd-5e4993a1c071.png)

![screenshot_3](https://user-images.githubusercontent.com/24377109/79051882-f3e06580-7c32-11ea-974f-414f68394190.png)

![screenshot_4](https://user-images.githubusercontent.com/24377109/79051883-f5119280-7c32-11ea-801c-1595709d8060.png)
