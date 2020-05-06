#pragma once

namespace Io {
    // Glue interface between Game and Platform-specific controller
    class IMouseController {
    public:
        virtual int GetCursorX() const = 0;
        virtual int GetCursorY() const = 0;
    };
}
