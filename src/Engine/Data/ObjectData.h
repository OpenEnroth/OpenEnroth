#pragma once

#include <cstdint>

#include "ObjectEnums.h"

#include "Engine/Objects/SpriteEnums.h" // TODO(captainurist): Data -> Objects dependency, we don't want that.

#include "Core/Time/Duration.h"

#include "Library/Color/Color.h"

/**
 * A kind of `SpriteObject`, a row of `dobjlist.bin`. Covers items lying on the ground, arrows and monster bolts,
 * spell projectiles, impacts and explosions, and traps. Items are the only pickable rows.
 *
 * `SpriteObject::uObjectDescID` indexes these rows, and `ObjectTable::ObjectIDByItemID` finds a row by its `SpriteId`.
 */
struct ObjectData {
    inline bool NoSprite() const { return uFlags & OBJECT_DESC_NO_SPRITE; }

    SpriteId uObjectID = SPRITE_NULL;
    int16_t uRadius = 0;
    int16_t uHeight = 0;
    ObjectDescFlags uFlags;
    uint16_t uSpriteID = 0;
    Duration uLifetime;
    Color uParticleTrailColor;
    int16_t uSpeed = 0;
};
