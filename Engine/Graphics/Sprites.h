#pragma once

#include <string>

#include "Engine/OurMath.h"

#include "Engine/Graphics/DecorationList.h"

class Texture;

class Sprite {
 public:
    inline Sprite() {
        uPaletteID = 0;
        texture = nullptr;
        uAreaX = 0;
        uAreaY = 0;
        uBufferWidth = 0;
        uBufferHeight = 0;
        uAreaWidth = 0;
        uAreaHeight = 0;
        sprite_header = nullptr;
    }

    void Release();

    std::string pName;
    int uPaletteID;
    Texture *texture;
    int uAreaX;
    int uAreaY;
    int uBufferWidth;   // hardware width  (as opposed to LODSprite::Width)
    int uBufferHeight;  // hardware sprite height
    int uAreaWidth;
    int uAreaHeight;

    struct LODSprite *sprite_header;
};

class SpriteFrame {
 public:
    bool Deserialize(const struct SpriteFrame_MM7 *);

    std::string icon_name;
    std::string texture_name;

    Sprite* hw_sprites[8] {};
    float scale = 1.0;
    int uFlags = 0;  // 128 for loaded
    int uGlowRadius = 0;
    int uPaletteID = 0;
    int uPaletteIndex = 0;
    int uAnimTime = 0;
    int uAnimLength = 0;
};

#pragma pack(push, 1)
struct SpriteFrameTable {
    inline SpriteFrameTable() {
        uNumSpriteFrames = 0;
        pSpriteSFrames = nullptr;
        pSpritePFrames = nullptr;
        pSpriteEFrames = nullptr;
    }
    void ToFile();
    void FromFile(void *data_mm6, void *data_mm7, void *data_mm8);
    void ReleaseSFrames();
    void ResetLoadedFlags();
    void InitializeSprite(signed int uSpriteID);
    int FastFindSprite(const char *pSpriteName);
    int BinarySearch(const char *pSpriteName);
    SpriteFrame *GetFrame(unsigned int uSpriteID, unsigned int uTime);
    SpriteFrame *GetFrameBy_x(unsigned int uSpriteID, signed int a3);

    int uNumSpriteFrames;
    unsigned int uNumEFrames = 0;      // field_4;
    int unused_field = 0;              // field_8
    SpriteFrame *pSpriteSFrames;   // 0c  new SpriteFrame[X]
    SpriteFrame **pSpritePFrames;  // 10h  pSpritePFrames[i] = &pSpriteSFrames[pSpriteEFrames[i]];
    int16_t *pSpriteEFrames;       // 14h
};
#pragma pack(pop)

extern struct SpriteFrameTable *pSpriteFrameTable;

SpriteFrame *LevelDecorationChangeSeason(DecorationDesc *desc, int t, int month);
