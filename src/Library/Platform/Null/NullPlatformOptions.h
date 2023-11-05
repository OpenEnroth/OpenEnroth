#pragma once

#include <vector>

#include "Utility/Geometry/Point.h"
#include "Utility/Geometry/Size.h"
#include "Utility/Geometry/Margins.h"
#include "Utility/Geometry/Rect.h"

struct NullPlatformOptions {
    Pointi defaultWindowPosition = Pointi(0, 0);
    Sizei defaultWindowSize = Sizei(100, 100);
    Marginsi defaultFrameMargins = Marginsi(5, 5, 5, 5);
    std::vector<Recti> displayGeometries = {{0, 0, 1920, 1080}};
};
