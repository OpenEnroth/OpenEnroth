#pragma once

#include "Engine/VectorTypes.h"

struct SpawnPoint {
    Vec3i vPosition;
    uint16_t uRadius = 32;
    uint16_t uKind = 0; // 3 - monster, 2 - treasure.
    ITEM_TREASURE_LEVEL uItemIndex = ITEM_TREASURE_LEVEL_INVALID;
    uint16_t uMonsterIndex = 0;
    uint16_t uAttributes = 0;
    unsigned int uGroup = 0;

    inline bool IsMonsterSpawn() const { return uKind == 3; }
    inline bool IsTreasureSpawn() const { return uKind != 3; }
};
