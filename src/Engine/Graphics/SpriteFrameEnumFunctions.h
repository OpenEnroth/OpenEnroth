#pragma once

#include <cassert>

#include "SpriteFrameEnums.h"

/**
 * @returns                             Mirror flag for octant N: mirrorFlagForOctant(3) returns SPRITE_FRAME_MIRROR_3.
 */
constexpr SpriteFrameFlag mirrorFlagForOctant(int octant) {
    assert(octant >= 0 && octant <= 7);
    return static_cast<SpriteFrameFlag>(0x100 << octant);
}
