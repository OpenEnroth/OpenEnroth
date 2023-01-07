#include "SdlGamepad.h"

#include <cassert>

#include "SdlPlatformSharedState.h"

SdlGamepad::SdlGamepad(SdlPlatformSharedState *state, SDL_GameController *gamepad, uint32_t id):
    state_(state), gamepad_(gamepad), id_(id) {
    assert(state);
    assert(gamepad);
}

SdlGamepad::~SdlGamepad() {
    state_->UnregisterGamepad(this);
    SDL_GameControllerClose(gamepad_);
}

std::string SdlGamepad::Model() const {
    const char *model = SDL_GameControllerName(SdlHandle());
    if (model != NULL)
        return model;

    return {};
}

std::string SdlGamepad::Serial() const {
    // TODO: Just update SDL
#if SDL_VERSION_ATLEAST(2, 0, 14)
    const char *serial = SDL_GameControllerGetSerial(SdlHandle());
    if (serial != NULL)
        return serial;
#endif

    return {};
}

int32_t SdlGamepad::GamepadSdlId() {
    SDL_Joystick *joystick = SDL_GameControllerGetJoystick(SdlHandle());
    int32_t joystickId =  SDL_JoystickInstanceID(joystick);

    return joystickId;
}
