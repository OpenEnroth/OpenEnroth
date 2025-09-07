#include "SdlPlatform.h"

#include <SDL3/SDL.h>

#include <cassert>
#include <memory>
#include <vector>
#include <string>

#include "Library/Logger/Logger.h"

#include "SdlPlatformSharedState.h"
#include "SdlEventLoop.h"
#include "SdlWindow.h"
#include "SdlGamepad.h"
#include "SdlEnumTranslation.h"
#include "SdlFreeDeleter.h"

static void SDLCALL sdlLogCallback(void *userdata, int category, SDL_LogPriority priority, const char *message) {
    LogLevel level = translateSdlLogLevel(priority);
    if (category == SDL_LOG_CATEGORY_ASSERT)
        level = LOG_CRITICAL; // This is an assertion, damn it! But SDL issues these at SDL_LOG_PRIORITY_WARN.

    logger->log(SdlPlatformSharedState::logCategory(), level, "{}", message);
}

SdlPlatform::SdlPlatform() {
    assert(logger);

    _state = std::make_unique<SdlPlatformSharedState>();

    SDL_SetLogOutputFunction(&sdlLogCallback, _state.get());

    _initialized = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);
    if (!_initialized) {
        _state->logSdlError("SDL_Init");
        return;
    }

    _state->initializeGamepads();
}

SdlPlatform::~SdlPlatform() {
    SDL_Quit(); // Safe to call even if there were errors in initialization.
    _state.reset(); // Shared state destructor asserts that all windows & event loops are destroyed.
}

std::unique_ptr<PlatformWindow> SdlPlatform::createWindow() {
    if (!_initialized)
        return nullptr;

#if __ANDROID__
    // TODO(captainurist): SDL orientation code turned out to be buggy and works only before window creation,
    //                     hardcode only landscape modes there for now.
    if (!SDL_SetHintWithPriority(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight", SDL_HINT_OVERRIDE))
        _state->logSdlError("SDL_SetHintWithPriority");
#endif

    SDL_Window *window = SDL_CreateWindow("", 100, 100, SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL);
    if (!window) {
        _state->logSdlError("SDL_CreateWindow");
        return nullptr;
    }

    uint32_t id = SDL_GetWindowID(window);
    if (!id) {
        _state->logSdlError("SDL_GetWindowID");
        return nullptr;
    }

    std::unique_ptr<SdlWindow> result = std::make_unique<SdlWindow>(_state.get(), window, id);
    _state->registerWindow(result.get());
    return result;
}

std::unique_ptr<PlatformEventLoop> SdlPlatform::createEventLoop() {
    if (!_initialized)
        return nullptr;

    return std::make_unique<SdlEventLoop>(_state.get());
}

std::vector<PlatformGamepad *> SdlPlatform::gamepads() {
    return _state->allGamepads();
}

void SdlPlatform::setCursorShown(bool cursorShown) {
    if (!_initialized)
        return;

    if (cursorShown) {
        if (!SDL_ShowCursor())
            _state->logSdlError("SDL_ShowCursor");
    } else {
        if (!SDL_HideCursor())
            _state->logSdlError("SDL_ShowCursor");
    }
}

bool SdlPlatform::isCursorShown() const {
    if (!_initialized)
        return true;

    return SDL_CursorVisible();
}

std::vector<Recti> SdlPlatform::displayGeometries() const {
    if (!_initialized)
        return {};

    int count = 0;
    std::unique_ptr<SDL_DisplayID[], SdlFreeDeleter> displays(SDL_GetDisplays(&count));
    if (!displays) {
        _state->logSdlError("SDL_GetDisplays");
        return {};
    }

    std::vector<Recti> result;

    SDL_Rect rect;
    for (int i = 0; i < count; i++) {
        if (!SDL_GetDisplayBounds(displays[i], &rect)) {
            _state->logSdlError("SDL_GetDisplayBounds");
            return {};
        }

        result.emplace_back(rect.x, rect.y, rect.w, rect.h);
    }

    return result;
}

void SdlPlatform::showMessageBox(const std::string &title, const std::string &message) const {
    // No need to check _initialized here. From SDL docs:
    //     This function may be called at any time, even before SDL_Init()
    if (!SDL_ShowSimpleMessageBox(0, title.c_str(), message.c_str(), nullptr))
        _state->logSdlError("SDL_ShowSimpleMessageBox");
}

int64_t SdlPlatform::tickCount() const {
    // No need to check _initialized here either.
    // Looking at implementation in SDL, this function works even w/o a call to SDL_Init.
    return SDL_GetTicks();
}

std::unique_ptr<Platform> Platform::createStandardPlatform() {
    return std::make_unique<SdlPlatform>();
}
