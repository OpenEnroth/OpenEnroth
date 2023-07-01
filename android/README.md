# OpenEnroth for Android

This repository is just a build system of OpenEnroth for Android and OpenEnroth's main repository is here:

[Main repository of OpenEnroth](https://github.com/OpenEnroth/OpenEnroth)

OpenEnroth is open-source Might and Magic VI-VIII engine remake using original data & code.
Which allows playing original games with enhancements and modifications.

Discord
---------------
Join our discord channel to discuss, track progress or involve in development of this project.

[![Discord channel invite](https://img.shields.io/badge/chat-on%20discord-green.svg)](https://discord.gg/jRCyPtq) 

Getting Started
---------------
1. You will require a GoG or any older retail version of Might and Magic VII. Currently you can build project only on POSIX systems because only custom bash script for building FFmpeg is provided.
2. Generate keystore with key for signing APK and place path, key alias and password for it in your local gradle.properties (~/.gradle/gradle.properties) in these variables OPENENROTH_RELEASE_STORE_FILE, OPENENROTH_RELEASE_KEY_ALIAS and OPENENROTH_RELEASE_PASSWORD.
3. Go to `openenroth/jni/FFmpeg` and execute build.sh to build FFmpeg for various architectures.
4. Use Android Studio or invoke gradle manually from console to build APK file.
5. Install it on your Android device.
6. Copy assets from Might and Magic VII and OpenEnroth-specific resources from the 'resources' folder of the main repo to the `/sdcard/Android/data/org.openenroth.game/files`.
