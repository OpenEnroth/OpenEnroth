#pragma once

#include <SDL.h>

#include "Library/Platform/Interface/PlatformOpenGLContext.h"

class SdlPlatformSharedState;

class SdlOpenGLContext: public PlatformOpenGLContext {
 public:
    SdlOpenGLContext(SdlPlatformSharedState *state, SDL_Window *window, SDL_GLContext context);
    virtual ~SdlOpenGLContext();

    virtual bool bind() override;
    virtual bool unbind() override;
    virtual void *nativeHandle() override;
    virtual void swapBuffers() override;
    virtual void *getProcAddress(const char *name) override;

 private:
    SdlPlatformSharedState *_state = nullptr;
    SDL_Window *_window = nullptr;
    SDL_GLContext _context = nullptr;
};
