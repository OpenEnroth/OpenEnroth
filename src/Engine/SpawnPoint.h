#pragma once

#include "Engine/VectorTypes.h"

struct SpawnPoint {
    Vec3i vPosition;
    uint16_t uRadius = 32;
    uint16_t uKind = 0; // 3 - monster, 2 - treasure.
    uint16_t uIndex = 0;
    uint16_t uAttributes = 0;
    unsigned int uGroup = 0;

    inline bool IsMonsterSpawn() const { return uKind == 3; }
    inline bool IsTreasureSpawn() const { return uKind != 3; }
};
