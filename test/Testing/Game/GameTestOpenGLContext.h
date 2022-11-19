#pragma once

#include <memory>

#include "Platform/PlatformOpenGLContext.h"

#include "GameTestStateHandle.h"

class GameTestOpenGLContext: public PlatformOpenGLContext {
 public:
    GameTestOpenGLContext(std::unique_ptr<PlatformOpenGLContext> base, GameTestStateHandle state);

    virtual bool MakeCurrent() override;
    virtual void SwapBuffers() override;
    virtual void* GetProcAddress(const char* name) override;

 private:
    std::unique_ptr<PlatformOpenGLContext> base_;
    GameTestStateHandle state_;
};
