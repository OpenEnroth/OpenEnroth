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

void ObjectList::InitializeColors() {
    for (ObjectDesc &object : pObjects) {
        object.uParticleTrailColor =
            ((unsigned int)object.uParticleTrailColorB << 16) |
            ((unsigned int)object.uParticleTrailColorG << 8) |
            ((unsigned int)object.uParticleTrailColorR);
    }
}

void ObjectList::FromFile(const Blob &data_mm6, const Blob &data_mm7, const Blob &data_mm8) {
    pObjects.clear();

    if (data_mm6)
        Deserialize(data_mm6, appendVia<ObjectDesc_MM6>(&pObjects));
    if (data_mm7)
        Deserialize(data_mm7, appendVia<ObjectDesc_MM7>(&pObjects));
    if (data_mm8)
        Deserialize(data_mm8, appendVia<ObjectDesc_MM7>(&pObjects));

    assert(!pObjects.empty());
}
