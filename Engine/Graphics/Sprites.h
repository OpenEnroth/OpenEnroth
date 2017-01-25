#pragma once
#include "Engine/OurMath.h"

/*   18 */
#pragma pack(push, 1)
class Sprite  //28h
{
public:
    inline Sprite()
    {
    }
    ~Sprite();

    void Release();

    const char *pName;  //0
    int uPaletteID; //4
    //struct IDirectDrawSurface4 *pTextureSurface;  //8
    //struct IDirect3DTexture2 *pTexture;   //ch
    Texture *texture;
    int uAreaX;  //10h
    int uAreaY;  //14h
    int uBufferWidth;  //18h  hardware width  (as opposed to LODSprite::Width)
    int uBufferHeight;  //1ch  hardware sprite height
    int uAreaWidth;  //20h
    int uAreaHeight; //24h

    struct LODSprite *sprite_header;
    //struct ID3D11ShaderResourceView *d3d11_srv;
};
#pragma pack(pop)



class SpriteFrame
{
    public:
        bool Deserialize(const struct SpriteFrame_MM6 *);
        bool Deserialize(const struct SpriteFrame_MM7 *);

        String icon_name;
        String texture_name;

        Sprite *hw_sprites[8];
        fixed scale; // used in screen space scaling
        int uFlags;
        int uGlowRadius;
        int uPaletteID;
        int uPaletteIndex;
        int uAnimTime;
        int uAnimLength;
};


/*   43 */
#pragma pack(push, 1)
struct SpriteFrameTable
{
    //----- (0044D4BA) --------------------------------------------------------
    inline SpriteFrameTable()
    {
        uNumSpriteFrames = 0;
        pSpriteSFrames = nullptr;
        pSpritePFrames = nullptr;
        pSpriteEFrames = nullptr;
    }
    void ToFile();
    void FromFile(void *data_mm6, void *data_mm7, void *data_mm8);
    bool FromFileTxt(const char *Args);
    void ReleaseSFrames();
    void ResetSomeSpriteFlags();
    void InitializeSprite(signed int uSpriteID);
    signed int FastFindSprite(char *pSpriteName);
    int BinarySearch(const char *pSpriteName);
    SpriteFrame *GetFrame(unsigned int uSpriteID, unsigned int uTime);
    SpriteFrame *GetFrameBy_x(unsigned int uSpriteID, signed int a3);

    signed int uNumSpriteFrames;
    unsigned int uNumEFrames;//field_4;
    int unused_field;          //field_8
    SpriteFrame *pSpriteSFrames;  //0c
    SpriteFrame **pSpritePFrames; //10h
    __int16 *pSpriteEFrames; //14h
};
#pragma pack(pop)




void _46E26D_collide_against_sprites(signed int a1, signed int a2);


extern struct SpriteFrameTable *pSpriteFrameTable;