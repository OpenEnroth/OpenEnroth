#include "Engine/Objects/ObjectList.h"

#include "Engine/Graphics/Sprites.h"

#include "Engine/Serialization/LegacyImages.h"

struct ObjectList *pObjectList;

unsigned int ObjectList::ObjectIDByItemID(unsigned int uItemID) {
    for (size_t i = 0; i < pObjects.size(); i++)
        if (uItemID == pObjects[i].uObjectID)
            return i;
    return 0;
}

void ObjectList::InitializeSprites() {
    for (const ObjectDesc &object : pObjects) {
        pSpriteFrameTable->InitializeSprite(object.uSpriteID);
    }
}

void ObjectList::FromFile(const Blob &data_mm6, const Blob &data_mm7, const Blob &data_mm8) {
    pObjects.clear();

    if (data_mm6)
        deserialize(data_mm6, &pObjects, appendVia<ObjectDesc_MM6>());
    if (data_mm7)
        deserialize(data_mm7, &pObjects, appendVia<ObjectDesc_MM7>());
    if (data_mm8)
        deserialize(data_mm8, &pObjects, appendVia<ObjectDesc_MM7>());

    assert(!pObjects.empty());
}
