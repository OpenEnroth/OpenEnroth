#include "SdlPlatform.h"

#include <SDL.h>

#include <cassert>
#include <memory>
#include <utility>

#include "Platform/PlatformEventHandler.h"

#include "SdlPlatformSharedState.h"
#include "SdlEventLoop.h"
#include "SdlWindow.h"
#include "SdlLogger.h"
#include "SdlGamepad.h"

SdlPlatform::SdlPlatform(PlatformLogger *logger) {
    assert(logger);

    state_ = std::make_unique<SdlPlatformSharedState>(this, logger);

    initialized_ = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) == 0;
    if (!initialized_)
        state_->LogSdlError("SDL_Init");
}

SdlPlatform::~SdlPlatform() {
    SDL_Quit(); // Safe to call even if there were errors in initialization.
}

std::unique_ptr<PlatformWindow> SdlPlatform::CreateWindow() {
    if (!initialized_)
        return nullptr;

#if __ANDROID__
    // TODO: SDL orientation code turned out to be buggy and works only before window creation, hardcode only landscape modes there for now.
    SDL_SetHintWithPriority(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight", SDL_HINT_OVERRIDE);
#endif

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

std::string SdlPlatform::StoragePath(const PLATFORM_STORAGE type) const {
    std::string result{};
    const char *path = NULL;

    switch (type) {
#if __ANDROID__
        case (ANDROID_STORAGE_INTERNAL):
            path = SDL_AndroidGetInternalStoragePath();
            if (path)
                result = path;
            break;
        case (ANDROID_STORAGE_EXTERNAL):
            path = SDL_AndroidGetExternalStoragePath();
            if (path)
                result = path;
            break;
#endif
        default:
            break;
    }

    return result;
}

std::unique_ptr<PlatformGamepad> SdlPlatform::CreateGamepad(uint32_t id) {
    if (!initialized_)
        return nullptr;

    SDL_GameController *gamepad = SDL_GameControllerOpen(id);
    if (!gamepad) {
        state_->LogSdlError("SDL_GameControllerOpen");
        return nullptr;
    }

    std::unique_ptr<SdlGamepad> result = std::make_unique<SdlGamepad>(state_.get(), gamepad, id);
    state_->RegisterGamepad(result.get());
    return result;
}
