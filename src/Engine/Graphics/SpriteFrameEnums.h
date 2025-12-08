#pragma once

#include "Utility/Flags.h"

// TODO(captainurist): Should be split in two, see FrameFlags.
//                     Also, I'm not sure the value naming is correct, some of the flags aren't even used.

/**
 * @see https://github.com/GrayFace/MMExtension/blob/a2ab9b12705de7576aecf8111168666d6398f830/Scripts/Structs/01%20common%20structs.lua#L2412
 */
enum class SpriteFrameFlag : int {
    SPRITE_FRAME_HAS_MORE = 0x1,        // Animation continues with more frames.
    SPRITE_FRAME_LUMINOUS = 0x2,        // Self-lit sprite.
    SPRITE_FRAME_FIRST = 0x4,           // First frame of animation.
    SPRITE_FRAME_IMAGE1 = 0x10,         // Single image for all 8 octants.
    SPRITE_FRAME_CENTER = 0x20,         // Center sprite.
    SPRITE_FRAME_FIDGET = 0x40,         // Part of a monster fidget sequence.
    SPRITE_FRAME_LOADED = 0x80,         // Sprite group has been loaded.
    SPRITE_FRAME_MIRROR_0 = 0x100,      // Mirror octant 0.
    SPRITE_FRAME_MIRROR_1 = 0x200,      // Mirror octant 1.
    SPRITE_FRAME_MIRROR_2 = 0x400,      // Mirror octant 2.
    SPRITE_FRAME_MIRROR_3 = 0x800,      // Mirror octant 3.
    SPRITE_FRAME_MIRROR_4 = 0x1000,     // Mirror octant 4.
    SPRITE_FRAME_MIRROR_5 = 0x2000,     // Mirror octant 5.
    SPRITE_FRAME_MIRROR_6 = 0x4000,     // Mirror octant 6.
    SPRITE_FRAME_MIRROR_7 = 0x8000,     // Mirror octant 7.
    SPRITE_FRAME_IMAGES3 = 0x10000,     // Only views 0,2,4 exist (mirrored). MM7+.
    SPRITE_FRAME_GLOW = 0x20000,        // Glow effect. MM7+.
    SPRITE_FRAME_TRANSPARENT = 0x40000, // Transparent sprite. MM7+. // TODO(captainurist): Is it? Looks like mirror_all flag.
};
using enum SpriteFrameFlag;
MM_DECLARE_FLAGS(SpriteFrameFlags, SpriteFrameFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(SpriteFrameFlags)
