#pragma once

#include "IO/IUserInputProvider.h"

class Sdl2UserInputProvider : public IUserInputProvider {
public:
    bool IsKeyPressed(GameKey key) const override;
    bool IsKeyHeld(GameKey key) const override;
};