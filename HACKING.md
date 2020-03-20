
Build from repository
---------------------

This project uses the [CMake](https://cmake.org) build system.
Use the following commands to build (it is recommended to build in a separate directory as shown here):

``` shell
$ mkdir build && cd build
$ cmake ..
$ make
```
or select platform dependent [generator](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html) for your favorite IDE.


This has been tested on Visual Studio 2019 - your experiences with other IDE's may vary.


Dependencies
------------

* [SDL2](https://www.libsdl.org/download-2.0.php) - crossplatform media framework
* [FFmpeg](https://ffmpeg.zeranoe.com/builds/) - video support
* [OpenAL](https://www.openal.org/downloads/OpenAL11CoreSDK.zip) - audio support
* [zlib](http://gnuwin32.sourceforge.net/packages/zlib.htm) - compression

These are now auto-resolved during CMake build.


Instructions
------------
You will require the following installed on your computer: Git, CMake and Visual Studio 2019.

* Clone, fork or download the repo "https://github.com/gp-alex/world-of-might-and-magic.git"
* Move to folder and create a seperate build directory
* Open CMake GUI and select the source directory and the build directory
* Configure -> Select "Visual Studio 16 2019" for the generator and "Win32" as the optional platform
* Generate
* Open Project (Can now close CMake)
* In Visual Studio 2019 Build the project
* Once that completes set "World_of_Might_and_Magic" as the startup project and run

NB - Always use a new empty folder for the build directory if you need to re-run CMake

Still having problems? Try the discord chat [![](https://img.shields.io/badge/chat-on%20discord-green.svg)](https://discord.gg/jRCyPtq)


Coding style
------------
For the C++ code we are following the [Google C++ Style Guide](http://google.github.io/styleguide/cppguide.html).<br />
Source code is automatically checked against it, and Pull Request will fail if you don't follow it.<br />
For style check on Windows platform, you can use [Visual Studio Code cpplint plugin](https://marketplace.visualstudio.com/items?itemName=mine.cpplint).
