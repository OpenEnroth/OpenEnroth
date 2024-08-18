# OpenEnroth for Android

OpenEnroth for Android is looking for a maintainer. If you want to build OpenEnroth for Android manually – you are on your own. The instructions in this file are not guaranteed to work.

Getting Started
---------------
1. You will require a GoG or any older retail version of Might and Magic VII.
2. Generate keystore file for signing the APK. Place keystore path, alias and passwords into env variables SIGNING_KEYSTORE, SIGNING_STORE_PASSWORD, SIGNING_KEY_ALIAS and SIGNING_KEY_PASSWORD.
3. Use Android Studio or invoke gradle manually from console to build the APK file.
4. Install it on your Android device.
5. Copy assets from Might and Magic VII to `/sdcard/Android/data/org.openenroth.game/files`.
