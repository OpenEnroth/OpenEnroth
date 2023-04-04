#include "Engine/Objects/ObjectList.h"

#include "Engine/Graphics/Sprites.h"

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

// TODO(caprainurist): this belongs to LegacyImages.h
#pragma pack(push, 1)
struct ObjectDesc_mm6 {
    inline bool NoSprite() const { return uFlags & OBJECT_DESC_NO_SPRITE; }

    char field_0[32];
    int16_t uObjectID;
    int16_t uRadius;
    int16_t uHeight;
    int16_t uFlags;
    uint16_t uSpriteID;
    int16_t uLifetime;
    uint16_t uParticleTrailColor;
    int16_t uSpeed;
    char uParticleTrailColorR;
    char uParticleTrailColorG;
    char uParticleTrailColorB;
    char field_35_clr;
};
#pragma pack(pop)

void ObjectList::FromFile(const Blob &data_mm6, const Blob &data_mm7, const Blob &data_mm8) {
    static_assert(sizeof(ObjectDesc_mm6) == 52, "Wrong type size");
    static_assert(sizeof(ObjectDesc) == 56, "Wrong type size");

    unsigned int num_mm6_objs = data_mm6 ? *(uint32_t*)data_mm6.data() : 0;
    unsigned int num_mm7_objs = data_mm7 ? *(uint32_t*)data_mm7.data() : 0;
    unsigned int num_mm8_objs = data_mm8 ? *(uint32_t*)data_mm8.data() : 0;

    size_t uNumObjects = num_mm6_objs + num_mm7_objs + num_mm8_objs;
    assert(uNumObjects != 0);
    assert(num_mm8_objs == 0);

    pObjects.resize(uNumObjects);
    memcpy(pObjects.data(), (char*)data_mm7.data() + 4, num_mm7_objs * sizeof(ObjectDesc));
    for (unsigned int i = 0; i < num_mm6_objs; ++i) {
        auto src = (ObjectDesc_mm6 *)((char *)data_mm6.data() + 4) + i;
        ObjectDesc *dst = &pObjects[num_mm7_objs + i];
        memcpy(dst->field_0, src->field_0, sizeof(dst->field_0));
        dst->uObjectID = src->uObjectID;
        dst->uRadius = src->uRadius;
        dst->uHeight = src->uHeight;
        dst->uFlags = OBJECT_DESC_FLAGS(src->uFlags);
        dst->uSpriteID = src->uSpriteID;
        dst->uLifetime = src->uLifetime;
        dst->uParticleTrailColor = src->uParticleTrailColor;
        dst->uSpeed = src->uSpeed;
        dst->uParticleTrailColorR = src->uParticleTrailColorR;
        dst->uParticleTrailColorG = src->uParticleTrailColorG;
        dst->uParticleTrailColorB = src->uParticleTrailColorB;
        dst->field_35_clr = src->field_35_clr;
        dst->field_36_clr = 0;
        dst->field_37_clr = 0;
    }
}
