#pragma once

#include "Platform/PlatformKey.h"

namespace Io {
    // Glue interface between Game and Platform-specific controller
    class IKeyboardController {
     public:
        virtual bool IsKeyPressed(PlatformKey key) const = 0;
        virtual bool IsKeyHeld(PlatformKey key) const = 0;
    };
}
