#pragma once

class PlatformOpenGLContext {
 public:
    virtual ~PlatformOpenGLContext() = default;

    virtual bool bind() = 0;
    virtual bool unbind() = 0;
    virtual void *nativeHandle() = 0;
    virtual void swapBuffers() = 0;

    virtual void *getProcAddress(const char *name) = 0;
};
