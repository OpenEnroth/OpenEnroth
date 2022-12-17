#pragma once

#include <memory>

#include "Platform/PlatformOpenGLContext.h"

#include "TestStateHandle.h"

class TestOpenGLContext: public PlatformOpenGLContext {
 public:
    TestOpenGLContext(std::unique_ptr<PlatformOpenGLContext> base, TestStateHandle state);

    virtual bool MakeCurrent() override;
    virtual void SwapBuffers() override;
    virtual void* GetProcAddress(const char* name) override;

 private:
    std::unique_ptr<PlatformOpenGLContext> base_;
    TestStateHandle state_;
};
