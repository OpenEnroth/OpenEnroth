#pragma once

#include <SDL.h>

#include <memory>
#include <string>

#include "Platform/PlatformGamepad.h"

class SdlPlatformSharedState;

class SdlGamepad: public PlatformGamepad {
 public:
    SdlGamepad(SdlPlatformSharedState *state, SDL_GameController *gamepad, uint32_t id);
    virtual ~SdlGamepad();

    virtual std::string model() const override;
    virtual std::string serial() const override;

    uint32_t id() const override {
        return _id;
    }

    SDL_GameController *sdlHandle() const {
        return _gamepad;
    }

    int32_t gamepadSdlId();

 private:
    SdlPlatformSharedState *_state = nullptr;
    SDL_GameController *_gamepad = nullptr;
    uint32_t _id = 0;
};
