#pragma once

#include "Io/GameKey.h"

namespace Io {
    // Glue interface between Game and Platform-specific controller
    class IKeyboardController {
    public:
        virtual bool IsKeyPressed(GameKey key) const = 0;
        virtual bool IsKeyHeld(GameKey key) const = 0;
    };
}
