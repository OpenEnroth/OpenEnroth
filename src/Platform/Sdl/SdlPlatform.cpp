#include "SdlPlatform.h"

#include <SDL.h>

#include <cassert>
#include <utility>

#include "Platform/PlatformEventHandler.h"

#include "SdlPlatformSharedState.h"
#include "SdlEventLoop.h"
#include "SdlWindow.h"
#include "SdlLogger.h"

SdlPlatform::SdlPlatform(PlatformLogger *logger) {
    assert(logger);

    state_ = std::make_unique<SdlPlatformSharedState>(this, logger);

    initialized_ = SDL_Init(SDL_INIT_VIDEO) == 0;
    if (!initialized_)
        state_->LogSdlError("SDL_Init");
}

SdlPlatform::~SdlPlatform() {
    SDL_Quit(); // Safe to call even if there were errors in initialization.
}

std::unique_ptr<PlatformWindow> SdlPlatform::CreateWindow() {
    if (!initialized_)
        return nullptr;

    SDL_Window *window = SDL_CreateWindow("", 0, 0, 100, 100, SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL);
    if (!window) {
        state_->LogSdlError("SDL_CreateWindow");
        return nullptr;
    }

    uint32_t id = SDL_GetWindowID(window);
    if (!id) {
        state_->LogSdlError("SDL_GetWindowID");
        return nullptr;
    }

    std::unique_ptr<SdlWindow> result = std::make_unique<SdlWindow>(state_.get(), window, id);
    state_->RegisterWindow(result.get());
    return result;
}

std::unique_ptr<PlatformEventLoop> SdlPlatform::CreateEventLoop() {
    if (!initialized_)
        return nullptr;

    return std::make_unique<SdlEventLoop>(state_.get());
}

void SdlPlatform::SetCursorShown(bool cursorShown) {
    if (!initialized_)
        return;

    if (SDL_ShowCursor(cursorShown ? SDL_ENABLE : SDL_DISABLE) < 0)
        state_->LogSdlError("SDL_ShowCursor");
}

bool SdlPlatform::IsCursorShown() const {
    if (!initialized_)
        return true;

    int result = SDL_ShowCursor(SDL_QUERY);
    if (result < 0) {
        state_->LogSdlError("SDL_ShowCursor");
        return true;
    } else {
        return result == SDL_ENABLE;
    }
}

std::vector<Recti> SdlPlatform::DisplayGeometries() const {
    if (!initialized_)
        return {};

    int displays = SDL_GetNumVideoDisplays();
    if (displays < 0) {
        state_->LogSdlError("SDL_GetNumVideoDisplays");
        return {};
    }

    std::vector<Recti> result;

    SDL_Rect rect;
    for (int i = 0; i < displays; i++) {
        if (SDL_GetDisplayBounds(i, &rect) != 0) {
            state_->LogSdlError("SDL_GetDisplayBounds");
            return {};
        }

        result.emplace_back(rect.x, rect.y, rect.w, rect.h);
    }

    return result;
}

void SdlPlatform::ShowMessageBox(const std::string &message, const std::string& title) const {
    SDL_ShowSimpleMessageBox(0, title.c_str(), message.c_str(), nullptr);
}

int64_t SdlPlatform::TickCount() const {
    // TODO(captainurist): Just update SDL
#if SDL_VERSION_ATLEAST(2, 0, 18)
    return SDL_GetTicks64();
#else
    return SDL_GetTicks();
#endif
}

std::string SdlPlatform::WinQueryRegistry(const std::string &) const {
    return {};
}

