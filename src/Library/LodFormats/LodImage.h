#pragma once

#include "Library/Image/Image.h"
#include "Library/Image/Palette.h"

struct LodImage {
    GrayscaleImage image;
    Palette palette;
    bool zeroIsTransparent = false; // Supposedly means that zero palette entry should be treated as transparent.
                                    // This, however, is not respected by the LOD data. We have icons that have no
                                    // transparency but have this flag set to `true`, and icons that have transparency
                                    // that have this flag set to `false`.
};
