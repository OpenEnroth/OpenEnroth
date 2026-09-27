#include "ObjectTable.h"

#include "Engine/Graphics/Sprites.h"

ObjectTable *pObjectTable;

unsigned int ObjectTable::ObjectIDByItemID(SpriteId uItemID) {
    for (size_t i = 0; i < pObjects.size(); i++)
        if (uItemID == pObjects[i].uObjectID)
            return i;
    return 0;
}

void ObjectTable::InitializeSprites() {
    for (const ObjectData &object : pObjects) {
        pSpriteFrameTable->InitializeSprite(object.uSpriteID);
    }
}
