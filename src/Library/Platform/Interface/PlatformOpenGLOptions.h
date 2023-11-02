#pragma once

enum class PlatformVSyncMode {
    GL_VSYNC_NONE,
    GL_VSYNC_ADAPTIVE,
    GL_VSYNC_NORMAL
};
using enum PlatformVSyncMode;

enum class PlatformOpenGLProfile {
    GL_PROFILE_CORE,
    GL_PROFILE_COMPATIBILITY,
    GL_PROFILE_ES
};
using enum PlatformOpenGLProfile;

struct PlatformOpenGLOptions {
    int versionMajor = -1;
    int versionMinor = -1;
    PlatformOpenGLProfile profile = GL_PROFILE_CORE;

    PlatformVSyncMode vsyncMode = GL_VSYNC_ADAPTIVE;
    bool doubleBuffered = true;

    int depthBits = -1;
    int stencilBits = -1;
};
