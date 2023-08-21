#pragma once

#include "Platform/PlatformOpenGLContext.h"
#include "ProxyBase.h"

class PlatformOpenGLContext;

class ProxyOpenGLContext: public ProxyBase<PlatformOpenGLContext> {
 public:
    explicit ProxyOpenGLContext(PlatformOpenGLContext *base = nullptr);
    virtual ~ProxyOpenGLContext() = default;

    virtual bool bind() override;
    virtual bool unbind() override;
    virtual void swapBuffers() override;
    virtual void *getProcAddress(const char *name) override;
};
