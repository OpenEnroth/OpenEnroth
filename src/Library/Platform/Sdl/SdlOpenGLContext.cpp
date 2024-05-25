#include "SdlOpenGLContext.h"

#include <cassert>

#include "SdlPlatformSharedState.h"

SdlOpenGLContext::SdlOpenGLContext(SdlPlatformSharedState *state, SDL_Window *window, SDL_GLContext context): _state(state), _window(window), _context(context) {
    assert(state);
    assert(window);
    assert(context);
}

SdlOpenGLContext::~SdlOpenGLContext() {}

bool SdlOpenGLContext::bind() {
    bool succeeded = SDL_GL_MakeCurrent(_window, _context) == 0;

    if (!succeeded)
        _state->logSdlError("SDL_GL_MakeCurrent");

    return succeeded;
}

bool SdlOpenGLContext::unbind() {
    bool succeeded = SDL_GL_MakeCurrent(_window, nullptr) == 0;

    if (!succeeded)
        _state->logSdlError("SDL_GL_MakeCurrent");

    return succeeded;
}

void SdlOpenGLContext::swapBuffers() {
    SDL_GL_SwapWindow(_window);
}

void *SdlOpenGLContext::nativeHandle() {
    return &_context;
}

void *SdlOpenGLContext::getProcAddress(const char *name) {
    if (SDL_GL_GetCurrentWindow() != _window || SDL_GL_GetCurrentContext() != _context)
        if (!bind())
            return nullptr;

    return SDL_GL_GetProcAddress(name);
}
