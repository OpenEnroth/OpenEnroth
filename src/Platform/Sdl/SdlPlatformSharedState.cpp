#include "SdlPlatformSharedState.h"

#include <cassert>

#include "Utility/MapAccess.h"

#include "SdlWindow.h"
#include "SdlPlatform.h"
#include "SdlLogger.h"
#include "SdlGamepad.h"

SdlPlatformSharedState::SdlPlatformSharedState(PlatformLogger *logger): _logger(logger) {
    assert(logger);
}

SdlPlatformSharedState::~SdlPlatformSharedState() {
    assert(_windowById.empty()); // Platform should be destroyed after all windows.
}

void SdlPlatformSharedState::logSdlError(const char *sdlFunctionName) {
    // Note that we cannot use `SDL_Log` here because we have no guarantees on the actual type of the logger
    // that was passed in constructor.
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "SDL error in %s: %s.", sdlFunctionName, SDL_GetError());
    _logger->log(PLATFORM_LOG, LOG_ERROR, buffer);
}

void SdlPlatformSharedState::registerWindow(SdlWindow *window) {
    assert(!_windowById.contains(window->id()));
    _windowById[window->id()] = window;
}

void SdlPlatformSharedState::unregisterWindow(SdlWindow *window) {
    assert(_windowById.contains(window->id()));
    _windowById.erase(window->id());
}

std::vector<uint32_t> SdlPlatformSharedState::allWindowIds() const {
    std::vector<uint32_t> result;
    for (auto [id, _] : _windowById)
        result.push_back(id);
    return result;
}

SdlWindow *SdlPlatformSharedState::window(uint32_t id) const {
    assert(_windowById.contains(id));
    return ValueOr(_windowById, id, nullptr);
}

void SdlPlatformSharedState::initializeGamepads() {
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (!SDL_IsGameController(i))
            continue;

        initializeGamepad(i);
    }
}

SdlGamepad *SdlPlatformSharedState::initializeGamepad(int gamepadId) {
    assert(SDL_IsGameController(gamepadId));

    std::unique_ptr<SDL_GameController, void(*)(SDL_GameController *)> gamepad(SDL_GameControllerOpen(gamepadId), &SDL_GameControllerClose);
    if (!gamepad) {
        logSdlError("SDL_GameControllerOpen");
        return nullptr;
    }

    SDL_Joystick *joystick = SDL_GameControllerGetJoystick(gamepad.get());
    if (!joystick) {
        logSdlError("SDL_GameControllerGetJoystick");
        return nullptr;
    }

    SDL_JoystickID id = SDL_JoystickInstanceID(joystick);
    if (id < 0) {
        logSdlError("SDL_JoystickInstanceID");
        return nullptr;
    }
    assert(!_gamepadById.contains(id)); // However, the code below will work fine even if id is there.

    return _gamepadById.emplace(id, std::make_unique<SdlGamepad>(this, gamepad.release(), id)).first->second.get();
}

void SdlPlatformSharedState::deinitializeGamepad(SDL_JoystickID id) {
    assert(_gamepadById.contains(id));

    _gamepadById.erase(id);
}

std::vector<PlatformGamepad *> SdlPlatformSharedState::allGamepads() const {
    std::vector<PlatformGamepad *> result;
    for (const auto &[_, gamepad] : _gamepadById)
        result.push_back(gamepad.get());
    return result;
}

SdlGamepad *SdlPlatformSharedState::gamepad(SDL_JoystickID id) const {
    auto pos = _gamepadById.find(id);
    return pos == _gamepadById.end() ? nullptr : pos->second.get();
}
