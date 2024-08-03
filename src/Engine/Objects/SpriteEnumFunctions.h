#pragma once

#include <cassert>


#include "SpriteEnums.h"

inline SpriteId impactSprite(SpriteId projectile) {
    assert(std::to_underlying(projectile) % 5 == 0);
    return static_cast<SpriteId>(std::to_underlying(projectile) + 1);
}
