#pragma once

#include <SDL.h>

#include "Platform/PlatformOpenGLContext.h"

class SdlPlatformSharedState;

class SdlOpenGLContext: public PlatformOpenGLContext {
 public:
    SdlOpenGLContext(SdlPlatformSharedState *state, SDL_Window *window, SDL_GLContext context);
    virtual ~SdlOpenGLContext();

    virtual bool MakeCurrent() override;
    virtual void SwapBuffers() override;
    virtual void *GetProcAddress(const char *name) override;

 private:
    SdlPlatformSharedState *state_ = nullptr;
    SDL_Window *window_ = nullptr;
    SDL_GLContext context_ = nullptr;
};
