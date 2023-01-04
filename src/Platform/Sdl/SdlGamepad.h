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

    virtual std::string Model() const override;
    virtual std::string Serial() const override;

    uint32_t Id() const override {
        return id_;
    }

    SDL_GameController *SdlHandle() const {
        return gamepad_;
    }

    int32_t GamepadSdlId();

 private:
    SdlPlatformSharedState *state_ = nullptr;
    SDL_GameController *gamepad_ = nullptr;
    uint32_t id_ = 0;
};
