#include "SdlWindow.h"

#include <cassert>
#include <string>
#include <memory>

#include "Library/Platform/Interface/PlatformEventHandler.h"

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
    if (!SDL_SetWindowTitle(_window, title.c_str()))
        _state->logSdlError("SDL_SetWindowTitle");
}

std::string SdlWindow::title() const {
    return SDL_GetWindowTitle(_window);
}

void SdlWindow::setIcon(RgbaImageView image) {
    // Note that this doesn't copy the pixel data.
    SDL_Surface *icon = SDL_CreateSurfaceFrom(
        image.width(),
        image.height(),
        SDL_PIXELFORMAT_RGBA8888,
        const_cast<Color *>(image.pixels().data()),
        image.width() * 4
    );
    if (!icon) {
        _state->logSdlError("SDL_CreateSurfaceFrom");
        return;
    }

    if (!SDL_SetWindowIcon(_window, icon))
        _state->logSdlError("SDL_SetWindowIcon");
    SDL_DestroySurface(icon);
}

void SdlWindow::resize(const Sizei &size) {
    if (!SDL_SetWindowSize(_window, size.w, size.h))
        _state->logSdlError("SDL_SetWindowSize");
}

Sizei SdlWindow::size() const {
    Sizei result;
    if (!SDL_GetWindowSize(_window, &result.w, &result.h)) {
        _state->logSdlError("SDL_GetWindowSize");
        return {};
    }
    return result;
}

void SdlWindow::setPosition(const Pointi &pos) {
    if (!SDL_SetWindowPosition(_window, pos.x, pos.y))
        _state->logSdlError("SDL_SetWindowPosition");
}

Pointi SdlWindow::position() const {
    Pointi result;
    if (!SDL_GetWindowPosition(_window, &result.x, &result.y)) {
        _state->logSdlError("SDL_GetWindowPosition");
        return {};
    }
    return result;
}

void SdlWindow::setVisible(bool visible) {
    if (visible) {
        if (!SDL_ShowWindow(_window))
            _state->logSdlError("SDL_ShowWindow");
    } else {
        if (!SDL_HideWindow(_window))
            _state->logSdlError("SDL_HideWindow");
    }
}

bool SdlWindow::isVisible() const {
    return !(SDL_GetWindowFlags(_window) & SDL_WINDOW_HIDDEN);
}

void SdlWindow::setResizable(bool resizable) {
    if (!SDL_SetWindowResizable(_window, resizable))
        _state->logSdlError("SDL_SetWindowResizable");
}

bool SdlWindow::isResizable() const {
    return SDL_GetWindowFlags(_window) & SDL_WINDOW_RESIZABLE;
}

void SdlWindow::setWindowMode(PlatformWindowMode mode) {
    if (!SDL_SetWindowBordered(_window, mode == WINDOW_MODE_WINDOWED))
        _state->logSdlError("SDL_SetWindowBordered");
    if (!SDL_SetWindowFullscreen(_window, mode == WINDOW_MODE_FULLSCREEN_BORDERLESS))
        _state->logSdlError("SDL_SetWindowFullscreen");
}

PlatformWindowMode SdlWindow::windowMode() {
    uint32_t flags = SDL_GetWindowFlags(_window);
    if ((flags & SDL_WINDOW_FULLSCREEN) == SDL_WINDOW_FULLSCREEN)
        return WINDOW_MODE_FULLSCREEN_BORDERLESS;
    if ((flags & SDL_WINDOW_BORDERLESS) == SDL_WINDOW_BORDERLESS)
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

    if (!SDL_SetHintWithPriority(SDL_HINT_ORIENTATIONS, hints.c_str(), SDL_HINT_OVERRIDE))
        _state->logSdlError("SDL_SetHintWithPriority");
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
    if (!SDL_SetWindowMouseGrab(_window, grabsMouse))
        _state->logSdlError("SDL_SetWindowMouseGrab");
}

bool SdlWindow::grabsMouse() const {
    return SDL_GetWindowMouseGrab(_window);
}

void SdlWindow::setMouseRelative(bool mouseRelative) {
    SDL_SetWindowRelativeMouseMode(_window, mouseRelative);
}

bool SdlWindow::isMouseRelative() const {
    return SDL_GetWindowRelativeMouseMode(_window);
}

Marginsi SdlWindow::frameMargins() const {
    Marginsi result;
    if(!SDL_GetWindowBordersSize(_window, &result.top, &result.left, &result.bottom, &result.right)) {
        _state->logSdlError("SDL_GetWindowBordersSize");
        return Marginsi();
    }
    return result;
}

void *SdlWindow::nativeHandle() const {
    return _window;
}

void SdlWindow::activate() {
    if (!SDL_RaiseWindow(_window))
        _state->logSdlError("SDL_RaiseWindow");
}

void SdlWindow::warpMouse(Pointi position) {
    SDL_WarpMouseInWindow(_window, position.x, position.y);
}

std::unique_ptr<PlatformOpenGLContext> SdlWindow::createOpenGLContext(const PlatformOpenGLOptions &options) {
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

    bool success = SDL_GL_SetSwapInterval(vsyncValue);
    if (!success && options.vsyncMode == GL_VSYNC_ADAPTIVE)
        success = SDL_GL_SetSwapInterval(translatePlatformVSyncMode(GL_VSYNC_NORMAL)); // Retry with normal vsync.

    if (!success)
        _state->logSdlError("SDL_GL_SetSwapInterval"); // Not a critical error, we still return context in this case.

    return std::make_unique<SdlOpenGLContext>(_state, _window, ctx);
}
