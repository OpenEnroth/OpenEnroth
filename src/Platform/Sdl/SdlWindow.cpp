#include "SdlWindow.h"

#include <SDL_syswm.h>
#ifdef None
#undef None
#endif

#include <cassert>
#include <utility>

#include "Platform/PlatformEventHandler.h"

#include "SdlPlatformSharedState.h"
#include "SdlEnumTranslation.h"
#include "SdlOpenGLContext.h"

SdlWindow::SdlWindow(SdlPlatformSharedState *state, SDL_Window *window, uint32_t id): _state(state), _window(window), _id(id) {
    assert(state);
    assert(window);
    assert(id);
}

SdlWindow::~SdlWindow() {
    _state->unregisterWindow(this);
    SDL_DestroyWindow(_window);
}

void SdlWindow::setTitle(const std::string &title) {
    SDL_SetWindowTitle(_window, title.c_str());
}

std::string SdlWindow::title() const {
    return SDL_GetWindowTitle(_window);
}

void SdlWindow::resize(const Sizei &size) {
    SDL_SetWindowSize(_window, size.w, size.h);
}

Sizei SdlWindow::size() const {
    Sizei result;
    SDL_GetWindowSize(_window, &result.w, &result.h);
    return result;
}

void SdlWindow::setPosition(const Pointi &pos) {
    SDL_SetWindowPosition(_window, pos.x, pos.y);
}

Pointi SdlWindow::position() const {
    Pointi result;
    SDL_GetWindowPosition(_window, &result.x, &result.y);
    return result;
}

void SdlWindow::setVisible(bool visible) {
    if (visible) {
        SDL_ShowWindow(_window);
    } else {
        SDL_HideWindow(_window);
    }
}

bool SdlWindow::isVisible() const {
    uint32_t flags = SDL_GetWindowFlags(_window);
    if (flags & SDL_WINDOW_SHOWN)
        return true;
    if (flags & SDL_WINDOW_HIDDEN)
        return false;

    assert(false); // shouldn't get here.
    return false;
}

void SdlWindow::setResizable(bool resizable) {
    SDL_SetWindowResizable(_window, resizable ? SDL_TRUE : SDL_FALSE);
}

bool SdlWindow::isResizable() const {
    uint32_t flags = SDL_GetWindowFlags(_window);
    if (flags & SDL_WINDOW_RESIZABLE)
        return true;

    return false;
}

void SdlWindow::setWindowMode(PlatformWindowMode mode) {
    uint32_t flags = 0;

    if (mode == WINDOW_MODE_FULLSCREEN_BORDERLESS)
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    else if (mode == WINDOW_MODE_FULLSCREEN)
        flags |= SDL_WINDOW_FULLSCREEN;

    if (SDL_SetWindowFullscreen(_window, flags) != 0)
        _state->logSdlError("SDL_SetWindowFullscreen");

    if (mode == WINDOW_MODE_WINDOWED)
        SDL_SetWindowBordered(_window, SDL_TRUE);
    else if (mode == WINDOW_MODE_BORDERLESS)
        SDL_SetWindowBordered(_window, SDL_FALSE);
}

PlatformWindowMode SdlWindow::windowMode() {
    uint32_t flags = SDL_GetWindowFlags(_window);

    if ((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP)
        return WINDOW_MODE_FULLSCREEN_BORDERLESS;
    else if ((flags & SDL_WINDOW_FULLSCREEN) == SDL_WINDOW_FULLSCREEN)
        return WINDOW_MODE_FULLSCREEN;
    else if ((flags & SDL_WINDOW_BORDERLESS) > 0)
        return WINDOW_MODE_BORDERLESS;

    return WINDOW_MODE_WINDOWED;
}

void SdlWindow::setOrientations(PlatformWindowOrientations orientations) {
    std::string hints{};

    if (orientations & LANDSCAPE_LEFT)
        hints += "LandscapeLeft ";
    if (orientations & LANDSCAPE_RIGHT)
        hints += "LandscapeRight ";
    if (orientations & PORTRAIT_UP)
        hints += "Portrait ";
    if (orientations & PORTRAIT_DOWN)
        hints += "PortraitUpsideDown ";

    SDL_SetHintWithPriority(SDL_HINT_ORIENTATIONS, hints.c_str(), SDL_HINT_OVERRIDE);
}

PlatformWindowOrientations SdlWindow::orientations() {
    PlatformWindowOrientations orientations;

    if (SDL_GetHint("LandscapeLeft"))
        orientations |= LANDSCAPE_LEFT;
    if (SDL_GetHint("LandscapeRight"))
        orientations |= LANDSCAPE_RIGHT;
    if (SDL_GetHint("Portrait"))
        orientations |= PORTRAIT_UP;
    if (SDL_GetHint("PortraitUpsideDown"))
        orientations |= PORTRAIT_DOWN;

    return orientations;
}

void SdlWindow::setGrabsMouse(bool grabsMouse) {
    SDL_SetWindowGrab(_window, grabsMouse ? SDL_TRUE : SDL_FALSE);
}

bool SdlWindow::grabsMouse() const {
    return SDL_GetWindowGrab(_window) == SDL_TRUE;
}

Marginsi SdlWindow::frameMargins() const {
    Marginsi result;
    if(SDL_GetWindowBordersSize(_window, &result.top, &result.left, &result.bottom, &result.right) != 0) {
        _state->logSdlError("SDL_GetWindowBordersSize");
        return Marginsi();
    }
    return result;
}

uintptr_t SdlWindow::systemHandle() const {
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);

    if (SDL_GetWindowWMInfo(_window, &info) != SDL_TRUE) {
        _state->logSdlError("SDL_GetWindowWMInfo");
        return 0;
    }

#if SDL_VIDEO_DRIVER_WINDOWS
    return reinterpret_cast<uintptr_t>(info.info.win.window);
#elif SDL_VIDEO_DRIVER_X11
    return static_cast<uintptr_t>(info.info.x11.window);
#elif SDL_VIDEO_DRIVER_COCOA
    return reinterpret_cast<uintptr_t>(info.info.cocoa.window);
#elif SDL_VIDEO_DRIVER_ANDROID
    return reinterpret_cast<uintptr_t>(info.info.android.window);
#else
#   error "Unsupported SDL video driver."
#endif
}

void SdlWindow::activate() {
    SDL_RaiseWindow(_window);
}

std::unique_ptr<PlatformOpenGLContext> SdlWindow::createOpenGLContext(const PlatformOpenGLOptions &options) {
    int version;

    if (options.versionMajor != -1)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, options.versionMajor);

    if (options.versionMinor != -1)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, options.versionMinor);

    if (options.depthBits != -1)
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, options.depthBits);

    if (options.stencilBits != -1)
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, options.stencilBits);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, translatePlatformOpenGLProfile(options.profile));
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, options.doubleBuffered);

    SDL_GLContext ctx = SDL_GL_CreateContext(_window);
    if (!ctx) {
        _state->logSdlError("SDL_GL_CreateContext");
        return nullptr;
    }

    int vsyncValue = translatePlatformVSyncMode(options.vsyncMode);

    int status = SDL_GL_SetSwapInterval(vsyncValue);
    if (status < 0 && options.vsyncMode == GL_VSYNC_ADAPTIVE)
        status = SDL_GL_SetSwapInterval(translatePlatformVSyncMode(GL_VSYNC_NORMAL)); // Retry with normal vsync.

    if (status < 0)
        _state->logSdlError("SDL_GL_SetSwapInterval"); // Not a critical error, we still return context in this case.

    return std::make_unique<SdlOpenGLContext>(_state, _window, ctx);
}
