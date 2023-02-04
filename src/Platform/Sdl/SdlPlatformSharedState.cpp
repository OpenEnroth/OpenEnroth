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
    _logger->log(PlatformLog, LogError, buffer);
}

void SdlPlatformSharedState::registerWindow(SdlWindow *window) {
    assert(!_windowById.contains(window->Id()));
    _windowById[window->Id()] = window;
}

void SdlPlatformSharedState::unregisterWindow(SdlWindow *window) {
    assert(_windowById.contains(window->Id()));
    _windowById.erase(window->Id());
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

void SdlPlatformSharedState::registerGamepad(SdlGamepad *gamepad) {
    int32_t id = nextFreeGamepadId();
    assert(id >= 0);

    _gamepadById[id] = gamepad;
}

void SdlPlatformSharedState::unregisterGamepad(SdlGamepad *gamepad) {
    for (auto it = _gamepadById.begin(); it != _gamepadById.end(); it++) {
        if (gamepad == it->second) {
            _gamepadById.erase(it);
            return;
        }
    }

    assert(0); //shouldn't happen
}

int32_t SdlPlatformSharedState::getGamepadIdBySdlId(uint32_t id) {
    int32_t ret = -1;

    for (auto it = _gamepadById.begin(); it != _gamepadById.end(); it++) {
        SdlGamepad *gamepad = it->second;
        if (gamepad->gamepadSdlId() == id)
            return gamepad->id();
    }

    return ret;
}

SdlGamepad *SdlPlatformSharedState::gamepad(uint32_t id) const {
    assert(_gamepadById.contains(id));
    return ValueOr(_gamepadById, id, nullptr);
}

int32_t SdlPlatformSharedState::nextFreeGamepadId() {
    int32_t ret = -1;
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (!_gamepadById.contains(i)) {
            ret = i;
            break;
        }
    }

    return ret;
}
