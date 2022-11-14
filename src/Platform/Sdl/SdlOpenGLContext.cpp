#include "SdlOpenGLContext.h"

#include <cassert>

#include "SdlPlatformSharedState.h"

SdlOpenGLContext::SdlOpenGLContext(SdlPlatformSharedState *state, SDL_Window *window, SDL_GLContext context): state_(state), window_(window), context_(context) {
    assert(state);
    assert(window);
    assert(context);
}

SdlOpenGLContext::~SdlOpenGLContext() {}

bool SdlOpenGLContext::MakeCurrent() {
    bool succeeded = SDL_GL_MakeCurrent(window_, context_) != 0;

    if (!succeeded)
        state_->LogSdlError("SDL_GL_MakeCurrent");

    return succeeded;
}

void SdlOpenGLContext::SwapBuffers() {
    SDL_GL_SwapWindow(window_);
}

void *SdlOpenGLContext::GetProcAddress(const char* name) {
    if (SDL_GL_GetCurrentWindow() != window_ || SDL_GL_GetCurrentContext() != context_)
        if (!MakeCurrent())
            return nullptr;

    return SDL_GL_GetProcAddress(name);
}
