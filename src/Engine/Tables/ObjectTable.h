#pragma once

#include <vector>

#include "Engine/Data/ObjectData.h"
#include "Engine/Objects/SpriteEnums.h"

struct ObjectTable {
    void InitializeSprites();
    unsigned int ObjectIDByItemID(SpriteId uItemID);

    std::vector<ObjectData> pObjects;
};

extern ObjectTable *pObjectTable;
