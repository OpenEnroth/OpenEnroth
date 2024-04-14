#pragma once

#include <Library/Geometry/Rect.h>

class UiRenderer {
 public:
    virtual void render(int framebuffer) = 0;
    virtual void setViewport(int width, int height) = 0;
};
