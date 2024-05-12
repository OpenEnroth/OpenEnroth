#pragma once

#include "Library/Platform/Interface/PlatformOpenGLContext.h"

class NullPlatformSharedState;

class NullOpenGLContext : public PlatformOpenGLContext {
 public:
    explicit NullOpenGLContext(NullPlatformSharedState *state);
    virtual ~NullOpenGLContext();

    virtual bool bind() override;
    virtual bool unbind() override;
    virtual void *nativeHandle() override;
    virtual void swapBuffers() override;
    virtual void *getProcAddress(const char *name) override;

 private:
    NullPlatformSharedState *_state = nullptr;
};
