# Might and Magic Trilogy [![](https://img.shields.io/badge/chat-on%20discord-green.svg)](https://discord.gg/jRCyPtq) 
![windows](https://github.com/gp-alex/world-of-might-and-magic/workflows/Windows_CI/badge.svg)
![linux](https://github.com/gp-alex/world-of-might-and-magic/workflows/Linux_CI/badge.svg)
Might and Magic VI-VIII engine remake using original data & code

![alt text](https://raw.githubusercontent.com/SourceCodeDeleted/world-of-might-and-magic/master/install_images/gamestart.PNG)



Getting Started
---------------
1. You will require a GoG or any older version of Might and Magic 7 installed for engine to guess game assets folder. You can override  assets/game path in launcher.
2. Use any IDE with CMake integration (Visual Studio is good).
2. Build & run game directly, or via launcher.

Getting Started (With IMAGES!)
--------------

<details><summary>Images</summary>
<p>

#### Set up cache
![alt text](https://raw.githubusercontent.com/SourceCodeDeleted/world-of-might-and-magic/master/install_images/install1.png)


#### Select Start Launch
![alt text](https://raw.githubusercontent.com/SourceCodeDeleted/world-of-might-and-magic/master/install_images/install2.png)

### wait until The cache has completed creating. (takes a couple of seconds)
![alt text](https://raw.githubusercontent.com/SourceCodeDeleted/world-of-might-and-magic/master/install_images/install3.png)

### Make sure your CMAKE conf looks like this (please use Visual Studio 2019 specifically, NINJA generator fails often)
![alt text](https://raw.githubusercontent.com/SourceCodeDeleted/world-of-might-and-magic/master/install_images/final.PNG)

### Click Build All
![alt text](https://raw.githubusercontent.com/SourceCodeDeleted/world-of-might-and-magic/master/install_images/install4.png)


</p>
</details>

Development
-----------
See the HACKING document in the source code root for information on how to compile.

GamePlay
--------------
![alt text](https://raw.githubusercontent.com/SourceCodeDeleted/world-of-might-and-magic/master/install_images/gameplay.PNG)

Current build status
--------------------
Master: [![Build Status](https://ci.appveyor.com/api/projects/status/nlno5vo74jf6rnt3/branch/master?svg=true)](https://ci.appveyor.com/project/gp-alex/world-of-might-and-magic)

Code analysis: [![Coverity Status](https://scan.coverity.com/projects/16434/badge.svg)](https://scan.coverity.com/projects/world-of-might-and-magic)
