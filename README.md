# Might and Magic Trilogy

[![Windows](https://github.com/OpenEnroth/OpenEnroth/workflows/Windows/badge.svg)](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/windows.yml) [![Linux](https://github.com/OpenEnroth/OpenEnroth/workflows/Linux/badge.svg)](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/linux.yml) [![MacOS](https://github.com/OpenEnroth/OpenEnroth/workflows/MacOS/badge.svg)](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/macos.yml) [![Doxygen](https://github.com/OpenEnroth/OpenEnroth/workflows/Doxygen/badge.svg)](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/doxygen.yml) [![Style Checker](https://github.com/OpenEnroth/OpenEnroth/workflows/Style/badge.svg)](https://github.com/OpenEnroth/OpenEnroth/actions/workflows/style.yml)

We are creating an extensible engine & modding environment that would make it possible to play original Might&Magic VI-VIII games on modern platforms with improved graphics and quality-of-life features expected of modern titles, and make modding and installing & playing the mods a pleasurable experience.

Currently only MM7 is playable. You can check out the [milestones](https://github.com/OpenEnroth/OpenEnroth/milestones) to see where we're at.

![screenshot_main](https://user-images.githubusercontent.com/24377109/79051217-491a7800-7c2f-11ea-85c7-f9120b7d79dd.png)

Discord
---------------
Join our discord channel to discuss, track progress or get involved in the development of this project.

[![Discord channel invite](https://img.shields.io/badge/chat-on%20discord-green.svg)](https://discord.gg/jRCyPtq) 

Getting Started
---------------
**1. You will require a GoG or any older version of Might and Magic 7**
<details><summary>&ensp;&ensp;details</summary>

* The game on GOG is provided only for windows. Game itself works fine also with linux and winr, but GOG installer (few Kb) wrapper may not
* To workaround it, you can download the full game offline installer (~600MB)
* `wine  ~/Downloads/setup_might_and_magic_7_1.1_\(hotfix\)_\(10933\).exe ` then works
* default installation points to `~/.wine/drive_c/GOG\ Games/Might\ and\ Magic\ 7`
</details>

**2. Build as described: [HACKING.md](HACKING.md#dependencies)**
<details><summary>&ensp;&ensp;verify</summary>

```
    ...
    [ 99%] Built target OpenEnroth
    [ 99%] Built target lj_gen_vm_s
    [100%] Building C object thirdparty/luajit/luajit/src/CMakeFiles/luajit.dir/luajit.c.o
    [100%] Linking C executable luajit
    [100%] Built target luajit
    # ls -l build/ | grep OpenEnroth -A1 -B1
    -rw-r--r--.  1 you you   41831 Feb 16 10:12 Makefile
    -rwxr-xr-x.  1 you you 5547552 Feb 16 10:18 OpenEnroth
    drwxr-xr-x. 12 you you    4096 Feb 16 10:12 src
```
</details>

**3. Mandatory copy OpenEnroth-specific resources from the 'resources' folder of the repo to the game assets folder.**
  * `cp -rv  resources/shaders ~/.wine/drive_c/GOG\ Games/Might\ and\ Magic\ 7`
<details><summary>&ensp;&ensp;verify</summary>

```
     # ls -l ~/.wine/drive_c/GOG\ Games/Might\ and\ Magic\ 7 | grep shaders -A1 -B1
     drwxr-xr-x. 2 you you     4096 Feb 16 09:52 Saves
     drwxr-xr-x. 2 you you     4096 Feb 16 10:03 shaders
     -rw-r--r--. 1 you you   138752 Jul  5  2017 Smackw32.dll
```
</details>

**4. You *have to* set path to the game by using `OPENENROTH_MM7_PATH` env variable.**
**5. Run freshly built binary**
  * `OPENENROTH_MM7_PATH=~/.wine/drive_c/GOG\ Games/Might\ and\ Magic\ 7 build/OpenEnroth `
  * If you forget to copy content of resources or point to wrong directory, game complains identically in console



Development
---------------
See the [HACKING](HACKING.md) document for information on how to compile or if you intend to contribute.

Screenshots
---------------
![screenshot_1](https://user-images.githubusercontent.com/24377109/79051879-f04cde80-7c32-11ea-939d-1dcc97b46f5d.png)

![screenshot_2](https://user-images.githubusercontent.com/24377109/79051881-f17e0b80-7c32-11ea-82cd-5e4993a1c071.png)

![screenshot_3](https://user-images.githubusercontent.com/24377109/79051882-f3e06580-7c32-11ea-974f-414f68394190.png)

![screenshot_4](https://user-images.githubusercontent.com/24377109/79051883-f5119280-7c32-11ea-801c-1595709d8060.png)
