#pragma once

#include <cassert>

#include "SpriteEnums.h"

/**
 * @returns                             Mirror flag for octant N: mirrorFlagForOctant(3) returns SPRITE_FRAME_MIRROR_3.
 */
constexpr SpriteFrameFlag mirrorFlagForOctant(int octant) {
    assert(octant >= 0 && octant <= 7);
    return static_cast<SpriteFrameFlag>(0x100 << octant);
}

constexpr BillboardFlags billboardFlagsForSprite(SpriteFrameFlags flags, int octant) {
    BillboardFlags result;
    if (flags & SPRITE_FRAME_LIT)
        result |= BILLBOARD_LIT;
    if (flags & mirrorFlagForOctant(octant))
        result |= BILLBOARD_MIRRORED;
    if (flags & SPRITE_FRAME_TRANSPARENT)
        result |= BILLBOARD_TRANSPARENT;
    if (flags & SPRITE_FRAME_GLOWING)
        result |= BILLBOARD_GLOWING;
    return result;
}
