#pragma once

#include "Library/Platform/Interface/PlatformOpenGLContext.h"

#include "ProxyBase.h"

class ProxyOpenGLContext: public ProxyBase<PlatformOpenGLContext> {
 public:
    explicit ProxyOpenGLContext(PlatformOpenGLContext *base = nullptr);
    virtual ~ProxyOpenGLContext() = default;

    virtual bool bind() override;
    virtual bool unbind() override;
    virtual void *nativeHandle() override;
    virtual void swapBuffers() override;
    virtual void *getProcAddress(const char *name) override;
};
