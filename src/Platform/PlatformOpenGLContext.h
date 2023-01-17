#pragma once

class PlatformOpenGLContext {
 public:
    virtual ~PlatformOpenGLContext() = default;

    virtual bool MakeCurrent() = 0;

    virtual void SwapBuffers() = 0;

    virtual void *GetProcAddress(const char *name) = 0;
};
