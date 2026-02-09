#pragma once

#include "Engine/Objects/ItemEnums.h"
#include "Engine/Pid.h"

#include "Library/Geometry/Vec.h"

struct SpawnPoint {
    Vec3f position;
    uint16_t radius = 32;
    ObjectType type = OBJECT_None;
    ItemTreasureLevel treasureLevel = ITEM_TREASURE_LEVEL_INVALID;
    uint16_t monsterIndex = 0;
    uint16_t attributes = 0;
    unsigned int group = 0;
};
