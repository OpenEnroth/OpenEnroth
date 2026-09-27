#pragma once

#include <vector>

#include "Engine/Data/ObjectData.h"
#include "Engine/Objects/SpriteEnums.h"

struct ObjectTable {
    void InitializeSprites();
    unsigned int ObjectIDByItemID(SpriteId uItemID); // TODO(captainurist): rename to objectDataId, it takes a SpriteId, not an item id.

    std::vector<ObjectData> pObjects;
};

extern ObjectTable *pObjectTable;
