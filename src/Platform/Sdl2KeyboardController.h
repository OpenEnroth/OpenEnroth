#pragma once

#include "Io/IKeyboardController.h"


class Sdl2KeyboardController : public Io::IKeyboardController {
 public:
    bool IsKeyPressed(PlatformKey key) const override;
    bool IsKeyHeld(PlatformKey key) const override;
};
