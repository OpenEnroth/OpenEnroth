#pragma once

#include "Library/Image/Image.h"

struct LodSprite {
    GrayscaleImage image;
    int paletteId = 0; // Index of default palette for this sprite. Not used in the code, actual palette index is taken
                       // from `SpriteFrameTable`. Default palette for monster sprites usually has cyan / magenta for
                       // parts that are recolored, and this is obviously not the palette that's used in the game.
};
