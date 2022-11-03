#pragma once

#include "Engine/VectorTypes.h"
#include "Engine/Objects/ActorEnums.h"

struct SpawnPoint {
    Vec3i vPosition;
    uint16_t uRadius = 32;
    ObjectType uKind = OBJECT_Any;
    ITEM_TREASURE_LEVEL uItemIndex = ITEM_TREASURE_LEVEL_INVALID;
    uint16_t uMonsterIndex = 0;
    uint16_t uAttributes = 0;
    unsigned int uGroup = 0;
};
