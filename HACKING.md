# Building instructions and guidance

This document contains information about required dependencies and various guidance about compilation and development processes.

Dependencies
---------------
Main dependencies:
* [SDL2](https://www.libsdl.org/download-2.0.php) - crossplatform media framework
* [FFmpeg](https://ffmpeg.zeranoe.com/builds/) - video support (NB 5.x currently unsupported)
* [OpenAL](https://www.openal.org/downloads/OpenAL11CoreSDK.zip) - audio support
* [Zlib](http://gnuwin32.sourceforge.net/packages/zlib.htm) - compression

On windows, the above dependencies are resolved automatically during the cmake phase.

Additional dependencies:
* CMake 3.20.4+ (3.20.21032501-MSVC_2 from VS2019 won't work)
* Python 3.x (optional, for style checker functionality)

Minimum required compiler versions are as follows:
* Visual Studio 2022
* GCC 11
* Clang 13

The following IDEs have been tested and should work fine:
* Visual Studio (2022 or later)
* Visual Studio Code (2022 or later)
* CLion (2022 or later)

Building on \*nix platforms
---------------
This project uses the [CMake](https://cmake.org) build system.
Use the following commands to clone repository and build (it is recommended to build in a separate directory as shown here):

```
$ git clone https://github.com/gp-alex/world-of-might-and-magic.git
$ cd world-of-might-and-magic
$ mkdir build && cd build
$ cmake ..
$ make
```

To compile x86 build on x86_64 platform you can pass `-m32` via compiler flags to cmake.
For example instead of just `cmake ..` above execute `CFLAGS="-m32" CXXFLAGS="-m32" cmake ..`

You can also select platform dependent [generator](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html) for your favorite IDE.

Building on Windows
---------------
You will require the following installed on your computer: Git (`https://git-scm.com/download/win`) and Visual Studio 2022.

* Make sure you have Windows SDK v10.0.20348.0 or higher
* Clone, fork or download the repo `https://github.com/OpenEnroth/OpenEnroth`
* Setup Cmake:
  * either install standalone cmake from the official website,
  * or add Microsoft one (that's coming with the VS installation) to your PATH environment variable (e.g `c:\Program Files\Microsoft Visual Studio\2022<edition>\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin`) 
* Open the folder in Visual Studio
* Select build configuration (x32 or x64) and wait for CMake configuration to complete
* Select startup item as `World_of_Might_and_Magic.exe`
* Run!

If you wish you can also disable autoresolving main dependencies by turning off `PREBUILT_DEPENDENCIES` cmake option and pass your own dependencies source, e.g. via [vcpkg](https://github.com/microsoft/vcpkg) integration.

__Be aware__ that Visual Studio has a bug with git submodules not syncing between branches.
So when checking out the branch or switching to different branch you may need to run the following command manually: `git submodule update --init`

Project Resources
---------------
The Git repo contains some additional resources required for the engine to run.
Please copy the entire 'shaders' folder from the subdirectory 'resources' to the location of the game assets.
Please check the logger output for the required path if you are unsure.

Support
---------------
Still having problems? Try to ask for help on our discord. [![](https://img.shields.io/badge/chat-on%20discord-green.svg)](https://discord.gg/jRCyPtq)


Coding style
---------------
For the C++ code we are following the [Google C++ Style Guide](http://google.github.io/styleguide/cppguide.html).\
Source code is automatically checked against it and Pull Request will fail if you don't follow it.

To perform style check before pushing anything you can build `check_style` target. In Visual Studio you can do that by going to ***Solution Explorer → Change Views → CMake targets***. Right click and build `check_style`, errors will be listed in output.

Some additional style preferences that we follow, in no particular order:
* `*` and `&` in type declarations are preceded by a space. So it's `char *string`, and not `char* string`.
* Documentation should be in doxydoc format with `@` used for tags, and starting with `/**` comment introducer.
* Documentation should be written in English. There are some leftover comments in Russian throughout the codebase, feel free to translate them into English when you have a chance.
* Please leave original function offsets intact. If you have a chance move them to doxygen comments using offset alias (eg `@offset 0xCAFEBEEF.`)
* Use `enum class`es followed by `using enum` statements instead of ordinary `enum`s. This provides type safety without changing the syntax. For flags, use `Flags` class.
* Use `MM_` prefix for macro naming.
