#pragma once

#include <cstdint>

enum OBJECT_DESC_FLAGS {
    OBJECT_DESC_NO_SPRITE = 0x1,
    OBJECT_DESC_NO_COLLISION = 0x2,
    OBJECT_DESC_TEMPORARY = 0x4,
    OBJECT_DESC_SFT_LIFETIME = 0x8,
    OBJECT_DESC_UNPICKABLE = 0x10,
    OBJECT_DESC_NO_GRAVITY = 0x20,
    OBJECT_DESC_INTERACTABLE = 0x40,
    OBJECT_DESC_BOUNCE = 0x80,
    OBJECT_DESC_TRIAL_PARTICLE = 0x100,
    OBJECT_DESC_TRIAL_FIRE = 0x200,
    OBJECT_DESC_TRIAL_LINE = 0x400,
};

#pragma pack(push, 1)
struct ObjectDesc {
    inline bool NoSprite() const { return uFlags & OBJECT_DESC_NO_SPRITE; }

    char field_0[32];
    int16_t uObjectID;
    int16_t uRadius;
    int16_t uHeight;
    int16_t uFlags; // TODO: OBJECT_DESC_FLAGS
    uint16_t uSpriteID;
    int16_t uLifetime;
    uint32_t uParticleTrailColor;
    int16_t uSpeed;
    uint8_t uParticleTrailColorR;
    uint8_t uParticleTrailColorG;
    uint8_t uParticleTrailColorB;
    char field_35_clr;
    char field_36_clr;
    char field_37_clr;
};
#pragma pack(pop)

class ObjectList {
 public:
    inline ObjectList() : uNumObjects(0), pObjects(nullptr) {}

    void FromFile(void *data_mm6, void *data_mm7, void *data_mm8);
    void InitializeSprites();
    void InitializeColors();
    unsigned int ObjectIDByItemID(unsigned int uItemID);

 protected:
    unsigned int uNumObjects;

 public:
    struct ObjectDesc *pObjects;
};

extern ObjectList *pObjectList;
