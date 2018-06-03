
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

Some useful configure variables (set as environment variable or with `-D` command line option):

* `SDL2DIR` - path to SDL2 root directory
* `FFMPEGDIR` - path to FFmpeg root directory
* `OPENALDIR` - path to OpenAL root directory
* `ZLIBDIR` - path to zlib root directory

Dependencies
------------

* [SDL2](https://www.libsdl.org/download-2.0.php) - crossplatform media framework
* [FFmpeg](https://ffmpeg.zeranoe.com/builds/) - video support
* [OpenAL](https://www.openal.org/downloads/OpenAL11CoreSDK.zip) - audio support
* [zlib](http://gnuwin32.sourceforge.net/packages/zlib.htm) - compression

On Windows you may run `win/WinPrebuild.ps1 %some_folder_to_install%` PowerShell script for download and install dependencies.

Coding style
------------
For the C++ code we are following the [Google C++ Style Guide](http://google.github.io/styleguide/cppguide.html).<br />
Source code is automatically checked against it, and Pull Request will fail if you don't follow it.<br />
For style check on Windows platform, you can use [Visual Studio Code cpplint plugin](https://marketplace.visualstudio.com/items?itemName=mine.cpplint).
