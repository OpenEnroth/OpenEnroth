#pragma once

enum class PlatformVSyncMode {
    NoVSync,
    AdaptiveVSync,
    NormalVSync
};
using enum PlatformVSyncMode;

enum class PlatformOpenGLProfile {
    CoreProfile,
    CompatibilityProfile
};
using enum PlatformOpenGLProfile;

struct PlatformOpenGLOptions {
    int versionMajor = -1;
    int versionMinor = -1;
    PlatformOpenGLProfile profile = CoreProfile;

    PlatformVSyncMode vsyncMode = AdaptiveVSync;
    bool doubleBuffered = true;

    int depthBits = -1;
    int stencilBits = -1;
};
