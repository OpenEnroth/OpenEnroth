#pragma once

#include <memory>

#include "Platform/Proxy/ProxyOpenGLContext.h"

#include "TestStateHandle.h"

class TestOpenGLContext: public ProxyOpenGLContext {
 public:
    TestOpenGLContext(std::unique_ptr<PlatformOpenGLContext> base, TestStateHandle state);

    virtual void SwapBuffers() override;

 private:
    std::unique_ptr<PlatformOpenGLContext> base_;
    TestStateHandle state_;
};
