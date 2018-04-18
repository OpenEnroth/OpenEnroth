# Might and Magic Trilogy [![](https://img.shields.io/badge/chat-on%20discord-green.svg)](https://discord.gg/jRCyPtq) 
Might and Magic VI-VIII engine remake using original data & code

# Getting Started
1. Use one of the projects for Visual Studio in /Builds folder. Latest are most up-to-date.
2. Required libavcodec dependencies are located in required_dlls folder
3. On older releases you will probably also need fixed MSS32.dll if project crashes for you.
4. Compile & run.
5. Launcher will try to guess your game installation directory (GoG or older releases). You can override folder with game resources using GUI.

# Coding style
For the C++ code we are following the [Google C++ Style Guide](http://google.github.io/styleguide/cppguide.html).
Source code is automatically checked against it, and Pull Request will fail if you don't follow it.
For style check on Windows platform, you can use [Visual Studio Code cpplint plugin](https://marketplace.visualstudio.com/items?itemName=mine.cpplint).

# Current build status
Master: [![Build Status](https://ci.appveyor.com/api/projects/status/nlno5vo74jf6rnt3/branch/master?svg=true)](https://ci.appveyor.com/project/gp-alex/world-of-might-and-magic)
