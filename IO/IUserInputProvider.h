#pragma once

#include "IO/GameKey.h"

class IUserInputProvider {
    virtual bool IsKeyPressed(GameKey key) const = 0;
    virtual bool IsKeyHeld(GameKey key) const = 0;
};