#pragma once
#include "Engine/Lod/Lod.h"


#pragma pack(push, 1)
struct LODSpriteHeader {
    inline LODSpriteHeader() {
        uHeight = 0;
        uPaletteId = 0;
        word_1A = 0;
    }

    char pName[12]{};         // 0
    uint32_t uSpriteSize = 0;        // C
    uint16_t uWidth = 0;         // 10  SW width (as opposed to Sprite::BufferWidth)
    uint16_t uHeight = 0;        // 12  SW height
    uint16_t uPaletteId = 0;     // 14
    uint16_t word_16 = 0;        // 16
    uint16_t uTexturePitch = 0;  // 18
    uint16_t word_1A = 0;        // 1a  flags - 1024 delete bitmap
    uint32_t uDecompressedSize = 0;  // 1c
};
#pragma pack(pop)


#pragma pack(push, 1)
struct LODSprite : public LODSpriteHeader {
    ~LODSprite();

    void Release();
    struct SoftwareBillboard* _4AD2D1_overlays(struct SoftwareBillboard* a2, int a3);

    uint8_t* bitmap = nullptr;
};
#pragma pack(pop)


#define MAX_LOD_SPRITES 1500
class Sprite;

class LODFile_Sprites : public LOD::Lod {
public:
    LODFile_Sprites();
    virtual ~LODFile_Sprites();

    void DeleteSomeSprites();
    void DeleteSpritesRange(int uStartIndex, int uStopIndex);
    int _461397();
    void DeleteSomeOtherSprites();
    int LoadSpriteFromFile(LODSprite* pSpriteHeader, const std::string& filename);
    bool LoadSprites(const std::string& filename);
    int LoadSprite(const char* filename, unsigned int uPaletteID);
    void ReleaseLostHardwareSprites();
    void ReleaseAll();
    void MoveSpritesToVideoMemory();
    void _inlined_sub0();
    void _inlined_sub1();

    unsigned int uNumLoadedSprites;
    int field_ECA0;  // reserved sprites -522
    int field_ECA4;  // 2nd init sprites
    int field_ECA8;
    // int can_load_hardware_sprites;
    Sprite* pHardwareSprites;
    int field_ECB4;
};

extern LODFile_Sprites* pSprites_LOD;
