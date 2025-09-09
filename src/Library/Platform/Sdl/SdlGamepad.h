#pragma once

#include <SDL3/SDL.h>

#include <string>

#include "Library/Platform/Interface/PlatformGamepad.h"

class SdlPlatformSharedState;

class SdlGamepad: public PlatformGamepad {
 public:
    SdlGamepad(SdlPlatformSharedState *state, SDL_Gamepad *gamepad, SDL_JoystickID id);
    virtual ~SdlGamepad();

    virtual std::string model() const override;
    virtual std::string serial() const override;

    SDL_JoystickID id() {
        return _id;
    }

 private:
    SdlPlatformSharedState *_state = nullptr;
    SDL_Gamepad *_gamepad = nullptr;
    SDL_JoystickID _id = 0;
};
