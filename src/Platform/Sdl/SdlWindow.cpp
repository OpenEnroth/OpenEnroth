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

SdlWindow::SdlWindow(SdlPlatformSharedState *state, SDL_Window *window, uint32_t id):
    state_(state), window_(window), id_(id) {
    assert(state);
    assert(window);
    assert(id);
}

SdlWindow::~SdlWindow() {
    state_->UnregisterWindow(this);
    SDL_DestroyWindow(window_);
}

void SdlWindow::SetTitle(const std::string &title) {
    SDL_SetWindowTitle(window_, title.c_str());
}

std::string SdlWindow::Title() const {
    return SDL_GetWindowTitle(window_);
}

void SdlWindow::Resize(const Sizei &size) {
    SDL_SetWindowSize(window_, size.w, size.h);
}

Sizei SdlWindow::Size() const {
    Sizei result;
    SDL_GetWindowSize(window_, &result.w, &result.h);
    return result;
}

void SdlWindow::SetPosition(const Pointi &pos) {
    SDL_SetWindowPosition(window_, pos.x, pos.y);
}

Pointi SdlWindow::Position() const {
    Pointi result;
    SDL_GetWindowPosition(window_, &result.x, &result.y);
    return result;
}

void SdlWindow::SetVisible(bool visible) {
    if (visible) {
        SDL_ShowWindow(window_);
    } else {
        SDL_HideWindow(window_);
    }
}

bool SdlWindow::IsVisible() const {
    uint32_t flags = SDL_GetWindowFlags(window_);
    if (flags & SDL_WINDOW_SHOWN)
        return true;
    if (flags & SDL_WINDOW_HIDDEN)
        return false;

    assert(false); // shouldn't get here.
    return false;
}

void SdlWindow::SetResizable(bool resizable) {
    SDL_SetWindowResizable(window_, resizable ? SDL_TRUE : SDL_FALSE);
}

bool SdlWindow::Resizable() const {
    uint32_t flags = SDL_GetWindowFlags(window_);
    if (flags & SDL_WINDOW_RESIZABLE)
        return true;

    return false;
}

void SdlWindow::SetWindowMode(PlatformWindowMode mode) {
    uint32_t flags = 0;

    if (mode == WINDOW_MODE_FAKE_FULLSCREEN)
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    else if (mode == WINDOW_MODE_FULLSCREEN)
        flags |= SDL_WINDOW_FULLSCREEN;

    if (SDL_SetWindowFullscreen(window_, flags) != 0)
        state_->LogSdlError("SDL_SetWindowFullscreen");

    if (mode == WINDOW_MODE_WINDOWED)
        SDL_SetWindowBordered(window_, SDL_TRUE);
    else if (mode == WINDOW_MODE_BORDERLESS)
        SDL_SetWindowBordered(window_, SDL_FALSE);
}

PlatformWindowMode SdlWindow::WindowMode() {
    uint32_t flags = SDL_GetWindowFlags(window_);

    if ((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP)
        return WINDOW_MODE_FAKE_FULLSCREEN;
    else if ((flags & SDL_WINDOW_FULLSCREEN) == SDL_WINDOW_FULLSCREEN)
        return WINDOW_MODE_FULLSCREEN;
    else if ((flags & SDL_WINDOW_BORDERLESS) > 0)
        return WINDOW_MODE_BORDERLESS;

    return WINDOW_MODE_WINDOWED;
}

void SdlWindow::SetOrientations(PlatformWindowOrientations orientations) {
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

PlatformWindowOrientations SdlWindow::Orientations() {
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

void SdlWindow::SetGrabsMouse(bool grabsMouse) {
    SDL_SetWindowGrab(window_, grabsMouse ? SDL_TRUE : SDL_FALSE);
}

bool SdlWindow::GrabsMouse() const {
    return SDL_GetWindowGrab(window_) == SDL_TRUE;
}

Marginsi SdlWindow::FrameMargins() const {
    Marginsi result;
    if(SDL_GetWindowBordersSize(window_, &result.top, &result.left, &result.bottom, &result.right) != 0) {
        state_->LogSdlError("SDL_GetWindowBordersSize");
        return Marginsi();
    }
    return result;
}

uintptr_t SdlWindow::SystemHandle() const {
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);

    if (SDL_GetWindowWMInfo(window_, &info) != SDL_TRUE) {
        state_->LogSdlError("SDL_GetWindowWMInfo");
        return 0;
    }

#ifdef _WINDOWS
    return reinterpret_cast<uintptr_t>(info.info.win.window);
#elif __APPLE__
    return reinterpret_cast<uintptr_t>(info.info.cocoa.window);
#elif ANDROID
    return reinterpret_cast<uintptr_t>(info.info.android.window);
#else
    return static_cast<uintptr_t>(info.info.x11.window);
#endif
}

void SdlWindow::Activate() {
    SDL_RaiseWindow(window_);
}

std::unique_ptr<PlatformOpenGLContext> SdlWindow::CreateOpenGLContext(const PlatformOpenGLOptions &options) {
    int version;

    if (options.versionMajor != -1)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, options.versionMajor);

    if (options.versionMinor != -1)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, options.versionMinor);

    if (options.depthBits != -1)
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, options.depthBits);

    if (options.stencilBits != -1)
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, options.stencilBits);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, TranslatePlatformOpenGLProfile(options.profile));
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, options.doubleBuffered);

    SDL_GLContext ctx = SDL_GL_CreateContext(window_);
    if (!ctx) {
        state_->LogSdlError("SDL_GL_CreateContext");
        return nullptr;
    }

    int vsyncValue = TranslatePlatformVSyncMode(options.vsyncMode);

    int status = SDL_GL_SetSwapInterval(vsyncValue);
    if (status < 0 && options.vsyncMode == GL_VSYNC_ADAPTIVE)
        status = SDL_GL_SetSwapInterval(TranslatePlatformVSyncMode(GL_VSYNC_NORMAL)); // Retry with normal vsync.

    if (status < 0)
        state_->LogSdlError("SDL_GL_SetSwapInterval"); // Not a critical error, we still return context in this case.

    return std::make_unique<SdlOpenGLContext>(state_, window_, ctx);
}
