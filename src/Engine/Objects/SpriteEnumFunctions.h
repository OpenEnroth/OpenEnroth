#pragma once

#include <cassert>

#include "Utility/Workaround/ToUnderlying.h"

#include "SpriteEnums.h"

inline SPRITE_OBJECT_TYPE impactSprite(SPRITE_OBJECT_TYPE projectile) {
    assert(std::to_underlying(projectile) % 5 == 0);
    return static_cast<SPRITE_OBJECT_TYPE>(std::to_underlying(projectile) + 1);
}
