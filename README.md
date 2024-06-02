# Might and Magic Trilogy

[![Windows](https://github.com/OpenEnroth/OpenEnroth/workflows/Windows/badge.svg)](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/windows.yml) [![Linux](https://github.com/OpenEnroth/OpenEnroth/workflows/Linux/badge.svg)](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/linux.yml) [![MacOS](https://github.com/OpenEnroth/OpenEnroth/workflows/MacOS/badge.svg)](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/macos.yml) [![Doxygen](https://github.com/OpenEnroth/OpenEnroth/workflows/Doxygen/badge.svg)](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/doxygen.yml) [![Style Checker](https://github.com/OpenEnroth/OpenEnroth/workflows/Style/badge.svg)](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/style.yml)

We are creating an extensible engine & modding environment that would make it possible to play original Might&Magic VI-VIII games on modern platforms with improved graphics and quality-of-life features expected of modern titles, and make modding and installing & playing the mods a pleasurable experience.

Currently only MM7 is playable. You can check out the [milestones](https://github.com/OpenEnroth/OpenEnroth/milestones) to see where we're at.

![screenshot_main](https://user-images.githubusercontent.com/24377109/79051217-491a7800-7c2f-11ea-85c7-f9120b7d79dd.png)

Download
-------

To download the code without having to compile it we have our releases at [https://github.com/OpenEnroth/OpenEnroth/releases](https://github.com/OpenEnroth/OpenEnroth/releases) 

Note: Currently there are only the nightly builds which may have bugs.

Discord
-------

Join our discord channel to discuss, track progress or get involved in the development of this project.

[![Discord channel invite](https://img.shields.io/badge/chat-on%20discord-green.svg)](https://discord.gg/jRCyPtq) 


Getting Started on Windows
--------------------------

1. You will need a GoG or any other version of Might and Magic VII. Run the installer and install the game into a custom location like `C:\Games`. (If you install into Program Files, there will be compatibility issues between the MM7 and OpenEnroth save file locations).

2. Download one of the prebuilt [releases](https://github.com/OpenEnroth/OpenEnroth/releases) and unzip the files.

3. From the `dist` folder inside the zip file copy the `OpenEnroth.exe` and the `shaders` folder to the folder where the game is installed.

4. Run `OpenEnroth.exe`. If you forget to copy the `shaders` folder, you will see errors in the console.


Getting Started on Linux or Mac
-------------------------------

1. You will need a GoG or any other version of Might and Magic VII. If you have an offline GoG installer, then follow these steps:
   * Install `innoextract`. If you're on Ubuntu, `sudo apt-get install innoextract` should work. If you're on Mac then you'll need homebrew, and `brew install innoextract` should do the job.
   * Run `innoextract -e -d <target-folder> <mm7-gog-folder>/setup_mm_7.exe`, where `<target-folder>` is the folder where you want to have game data extracted from the mm7 installer.
   * Check the files in `<target-folder>`, it should now contain the `app` subfolder. This is where game assets were extracted into.

2. Build OpenEnroth as described in [HACKING.md](HACKING.md).

3. Copy OpenEnroth-specific resources from the `resources` folder of the repo to the folder where the game assets were extracted to:

    `cp -rv resources/shaders <target-folder>/app`

4. Set `OPENENROTH_MM7_PATH` env variable to point to the location of the game assets. You might also want to add the following line to your bash profile (e.g. `~/.profile` on Ubuntu or `~/.zshrc` on Mac):

    `export OPENENROTH_MM7_PATH="<target-folder>/app"`

5. Run the freshly built `OpenEnroth` binary.


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
