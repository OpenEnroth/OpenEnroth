#include "SdlGamepad.h"

#include <cassert>

#include "SdlPlatformSharedState.h"

SdlGamepad::SdlGamepad(SdlPlatformSharedState *state, SDL_GameController *gamepad, uint32_t id):
    _state(state), _gamepad(gamepad), _id(id) {
    assert(state);
    assert(gamepad);
}

SdlGamepad::~SdlGamepad() {
    _state->unregisterGamepad(this);
    SDL_GameControllerClose(_gamepad);
}

std::string SdlGamepad::model() const {
    const char *model = SDL_GameControllerName(sdlHandle());
    if (model != NULL)
        return model;

    return {};
}

std::string SdlGamepad::serial() const {
    // TODO: Just update SDL
#if SDL_VERSION_ATLEAST(2, 0, 14)
    const char *serial = SDL_GameControllerGetSerial(sdlHandle());
    if (serial != NULL)
        return serial;
#endif

    return {};
}

int32_t SdlGamepad::gamepadSdlId() {
    SDL_Joystick *joystick = SDL_GameControllerGetJoystick(sdlHandle());
    int32_t joystickId =  SDL_JoystickInstanceID(joystick);

    return joystickId;
}
