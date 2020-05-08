#pragma once

#include "Io/IKeyboardController.h"


using Io::GameKey;


class Sdl2KeyboardController : public Io::IKeyboardController {
public:
    bool IsKeyPressed(GameKey key) const override;
    bool IsKeyHeld(GameKey key) const override;
};