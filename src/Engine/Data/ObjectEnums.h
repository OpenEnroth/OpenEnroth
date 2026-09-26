#pragma once

#include <cstdint>

#include "Utility/Flags.h"

// TODO(captainurist): rename to ObjectDataFlag/OBJECT_DATA_*. Plain OBJECT_* is taken by ObjectType.
enum class ObjectDescFlag : int16_t {
    OBJECT_DESC_NO_SPRITE = 0x1,
    OBJECT_DESC_NO_COLLISION = 0x2,
    OBJECT_DESC_TEMPORARY = 0x4,
    OBJECT_DESC_SFT_LIFETIME = 0x8,
    OBJECT_DESC_UNPICKABLE = 0x10,
    OBJECT_DESC_NO_GRAVITY = 0x20,
    OBJECT_DESC_INTERACTABLE = 0x40,
    OBJECT_DESC_BOUNCE = 0x80,
    OBJECT_DESC_TRAIL_PARTICLE = 0x100,
    OBJECT_DESC_TRAIL_FIRE = 0x200,
    OBJECT_DESC_TRAIL_LINE = 0x400,
};
using enum ObjectDescFlag;
MM_DECLARE_FLAGS(ObjectDescFlags, ObjectDescFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(ObjectDescFlags)
