#include "SdlPlatformSharedState.h"

#include <cassert>
#include <memory>
#include <vector>

#include "Library/Logger/Logger.h"

#include "Utility/MapAccess.h"

#include "SdlWindow.h"
#include "SdlLogSource.h"
#include "SdlGamepad.h"

// Log category should be a global so that it's registered at program startup.
static constinit SdlLogSource globalSdlLogSource;
static LogCategory globalSdlLogCategory("sdl", &globalSdlLogSource);

SdlPlatformSharedState::SdlPlatformSharedState() = default;

SdlPlatformSharedState::~SdlPlatformSharedState() {
    assert(_windowById.empty()); // Platform should be destroyed after all windows.
    assert(_eventLoopCount == 0); // And all event loops!
}

void SdlPlatformSharedState::logSdlError(const char *sdlFunctionName) {
    const char *errorMessage = SDL_GetError();
    if (!errorMessage || *errorMessage == '\0') // Not sure if SDL_GetError can return nullptr, but it definitely can return an empty string.
        errorMessage = "No error";
    logger->error(logCategory(), "SDL error in {}: {}", sdlFunctionName, errorMessage);
}

const LogCategory &SdlPlatformSharedState::logCategory() {
    return globalSdlLogCategory;
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
    return valueOr(_windowById, id, nullptr);
}

void SdlPlatformSharedState::registerEventLoop(SdlEventLoop *) {
    _eventLoopCount++;
}

void SdlPlatformSharedState::unregisterEventLoop(SdlEventLoop *) {
    _eventLoopCount--;
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

    // TODO(captainurist): The assert below triggers with @pskelton's xbox controller, and it shouldn't trigger.
    // Figure out why it happens and either fix the bug in our logic, or add a comment here describing what's happening.
    // assert(!_gamepadById.contains(id));
    // if (SdlGamepad *result = _gamepadById[id].get())
    //    return result;

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
