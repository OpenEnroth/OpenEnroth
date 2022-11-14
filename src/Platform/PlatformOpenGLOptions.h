#pragma once

enum class VSyncMode {
    NoVSync,
    AdaptiveVSync,
    NormalVSync
};
using enum VSyncMode;

struct PlatformOpenGLOptions {
    int versionMajor = -1;
    int versionMinor = -1;

    VSyncMode vSyncMode = AdaptiveVSync;
    bool doubleBuffered = true;

    int depthBits = -1;
    int stencilBits = -1;
};
