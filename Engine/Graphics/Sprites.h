#pragma once


/*   18 */
#pragma pack(push, 1)
class Sprite  //28h
{
  public:
    inline Sprite()
    {
      d3d11_srv = nullptr;
    }
    ~Sprite();

    void Release();

    const char *pName;  //0
    int uPaletteID; //4
    struct IDirectDrawSurface4 *pTextureSurface;  //8
    struct IDirect3DTexture2 *pTexture;   //ch
    int uAreaX;  //10h
    int uAreaY;  //14h
    int uBufferWidth;  //18h
    int uBufferHeight;  //1ch
    int uAreaWidth;  //20h
    int uAreaHeight; //24h

    struct ID3D11ShaderResourceView *d3d11_srv;
};
#pragma pack(pop)


/*   42 */
#pragma pack(push, 1)
struct SpriteFrame_mm6
{
    char pIconName[12]; 
    char pTextureName[12]; //c
    __int16 pHwSpriteIDs[8]; //18h
    int scale; //28h
    int uFlags; //2c
    __int16 uGlowRadius; //30
    __int16 uPaletteID;  //32
    __int16 uPaletteIndex;
    __int16 uAnimTime;
    //__int16 uAnimLength;
    //__int16 _pad;
};

class SpriteFrame: public SpriteFrame_mm6
{
public:
    __int16 uAnimLength;
    __int16 _pad;
};
#pragma pack(pop)

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




void __fastcall _46E26D_collide_against_sprites(signed int a1, signed int a2);


extern struct SpriteFrameTable *pSpriteFrameTable;