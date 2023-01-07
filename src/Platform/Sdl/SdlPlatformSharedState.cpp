#include "SdlPlatformSharedState.h"

#include <cassert>

#include "Utility/MapAccess.h"

#include "SdlWindow.h"
#include "SdlPlatform.h"
#include "SdlLogger.h"
#include "SdlGamepad.h"

SdlPlatformSharedState::SdlPlatformSharedState(SdlPlatform *owner, PlatformLogger *logger): owner_(owner), logger_(logger) {
    assert(owner);
    assert(logger);
}

SdlPlatformSharedState::~SdlPlatformSharedState() {
    assert(windowById_.empty()); // Platform should be destroyed after all windows.
}

void SdlPlatformSharedState::LogSdlError(const char *sdlFunctionName) {
    // Note that we cannot use `SDL_Log` here because we have no guarantees on the actual type of the logger
    // that was passed in constructor.
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "SDL error in %s: %s.", sdlFunctionName, SDL_GetError());
    logger_->Log(PlatformLog, LogError, buffer);
}

void SdlPlatformSharedState::RegisterWindow(SdlWindow *window) {
    assert(!windowById_.contains(window->Id()));
    windowById_[window->Id()] = window;
}

void SdlPlatformSharedState::UnregisterWindow(SdlWindow *window) {
    assert(windowById_.contains(window->Id()));
    windowById_.erase(window->Id());
}

std::vector<uint32_t> SdlPlatformSharedState::AllWindowIds() const {
    std::vector<uint32_t> result;
    for (auto [id, _] : windowById_)
        result.push_back(id);
    return result;
}

SdlWindow *SdlPlatformSharedState::Window(uint32_t id) const {
    assert(windowById_.contains(id));
    return ValueOr(windowById_, id, nullptr);
}

void SdlPlatformSharedState::RegisterGamepad(SdlGamepad *gamepad) {
    int32_t id = NextFreeGamepadId();
    assert(id >= 0);

    gamepadById_[id] = gamepad;
}

void SdlPlatformSharedState::UnregisterGamepad(SdlGamepad *gamepad) {
    for (auto it = gamepadById_.begin(); it != gamepadById_.end(); it++) {
        if (gamepad == it->second) {
            gamepadById_.erase(it);
            return;
        }
    }

    assert(0); //shouldn't happen
}

int32_t SdlPlatformSharedState::GetGamepadIdBySdlId(uint32_t id) {
    int32_t ret = -1;

    for (auto it = gamepadById_.begin(); it != gamepadById_.end(); it++) {
        SdlGamepad *gamepad = it->second;
        if (gamepad->GamepadSdlId() == id)
            return gamepad->Id();
    }

    return ret;
}

SdlGamepad *SdlPlatformSharedState::Gamepad(uint32_t id) const {
    assert(gamepadById_.contains(id));
    return ValueOr(gamepadById_, id, nullptr);
}

int32_t SdlPlatformSharedState::NextFreeGamepadId() {
    int32_t ret = -1;
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (!gamepadById_.contains(i)) {
            ret = i;
            break;
        }
    }

    return ret;
}
