#include "SdlGamepad.h"

#include <cassert>
#include <string>

#include "SdlPlatformSharedState.h"

SdlGamepad::SdlGamepad(SdlPlatformSharedState *state, SDL_Gamepad *gamepad, SDL_JoystickID id): _state(state), _gamepad(gamepad), _id(id) {
    assert(state);
    assert(gamepad);
}

SdlGamepad::~SdlGamepad() {
    SDL_CloseGamepad(_gamepad);
}

std::string SdlGamepad::model() const {
    const char *model = SDL_GetGamepadName(_gamepad);
    if (model != NULL)
        return model;

    return {};
}

std::string SdlGamepad::serial() const {
    const char *serial = SDL_GetGamepadSerial(_gamepad);
    if (serial != NULL)
        return serial;
    return {};
}
