#pragma once

#include "Platform/PlatformOpenGLContext.h"

#include "ProxyBase.h"

class ProxyOpenGLContext: public ProxyBase<PlatformOpenGLContext> {
 public:
    explicit ProxyOpenGLContext(PlatformOpenGLContext *base = nullptr);
    virtual ~ProxyOpenGLContext() = default;

    virtual bool Bind() override;
    virtual bool Unbind() override;
    virtual void SwapBuffers() override;
    virtual void *GetProcAddress(const char *name) override;
};
