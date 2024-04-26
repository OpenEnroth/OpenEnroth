#pragma once

#include <Library/Geometry/Size.h>

class IDebugViewRenderer {
 public:
    virtual ~IDebugViewRenderer() = default;
    virtual void render(const Sizei& outputPresent, int &drawCalls) = 0;
    virtual void reloadShaders() = 0;
};
