#pragma once

#include <vector>

#include "Library/Geometry/Point.h"
#include "Library/Geometry/Size.h"
#include "Library/Geometry/Margins.h"
#include "Library/Geometry/Rect.h"

struct NullPlatformOptions {
    Pointi defaultWindowPosition = Pointi(0, 0);
    Sizei defaultWindowSize = Sizei(100, 100);
    Marginsi defaultFrameMargins = Marginsi(5, 5, 5, 5);
    std::vector<Recti> displayGeometries = {{0, 0, 1920, 1080}};
};
