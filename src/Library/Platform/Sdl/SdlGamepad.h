#pragma once

#include <SDL.h>

#include <string>

#include "Library/Platform/Interface/PlatformGamepad.h"

class SdlPlatformSharedState;

class SdlGamepad: public PlatformGamepad {
 public:
    SdlGamepad(SdlPlatformSharedState *state, SDL_GameController *gamepad, SDL_JoystickID id);
    virtual ~SdlGamepad();

    virtual std::string model() const override;
    virtual std::string serial() const override;

    SDL_JoystickID id() {
        return _id;
    }

 private:
    SdlPlatformSharedState *_state = nullptr;
    SDL_GameController *_gamepad = nullptr;
    SDL_JoystickID _id = 0;
};
