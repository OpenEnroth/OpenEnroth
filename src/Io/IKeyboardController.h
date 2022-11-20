#pragma once

#include "Platform/PlatformKey.h"

namespace Io {

    // Glue interface between Game and Platform-specific controller
    class IKeyboardController {
     public:
        virtual bool ConsumeKeyPress(PlatformKey key) = 0;
        virtual bool IsKeyDown(PlatformKey key) const = 0;
    };
}
