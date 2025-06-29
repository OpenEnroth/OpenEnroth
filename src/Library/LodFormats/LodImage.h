#pragma once

#include "Library/Image/Image.h"
#include "Library/Image/Palette.h"

struct LodImage {
    GrayscaleImage image;
    Palette palette;
    bool zeroIsTransparent = false; // Means that zero palette entry should be treated as transparent.
                                    // This, however, is sometimes overridden in user code.
};
