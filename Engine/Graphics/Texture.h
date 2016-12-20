#pragma once
#include "Engine/Engine.h"

#include <stdio.h>
#include <array>

enum IMAGE_FORMAT
{
    IMAGE_FORMAT_R5G6B5 = 0,
    IMAGE_FORMAT_A1R5G5B5,
    IMAGE_FORMAT_A8R8G8B8,

    IMAGE_NUM_FORMATS,
    IMAGE_INVALID_FORMAT = -1,
};


inline unsigned int IMAGE_FORMAT_BytesPerPixel(IMAGE_FORMAT format)
{
    switch (format)
    {
        case IMAGE_FORMAT_R5G6B5:   return 2;
        case IMAGE_FORMAT_A1R5G5B5: return 2;
        case IMAGE_FORMAT_A8R8G8B8: return 4;

        default:
            Error("Invalid format: %d", format);
            return 0;
    }
}


struct ImageLoader
{
    virtual bool Load(unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format) = 0;
};



class Image
{
    public:
        inline Image(bool lazy_initialization = true):
            lazy_initialization(lazy_initialization), initialized(false),
            width(0), height(0), native_format(IMAGE_INVALID_FORMAT),
            loader(nullptr)
        {
            for (unsigned int i = 0; i < IMAGE_NUM_FORMATS; ++i)
                pixels[i] = nullptr;
        }


        bool Image16bit_From_LOD(const wchar_t *name);
        bool ColorKey_From_LOD(const wchar_t *name, unsigned __int16 colorkey);
        bool Alpha_From_LOD(const wchar_t *name);

        bool PCX_From_IconsLOD(const wchar_t *name);
        bool PCX_From_NewLOD(const wchar_t *name);
        bool PCX_From_File(const wchar_t *filename);


        unsigned int  GetWidth();
        unsigned int  GetHeight();
        const void   *GetPixels(IMAGE_FORMAT format);

        bool Release();


    protected:
        bool         lazy_initialization;
        bool         initialized;
        ImageLoader *loader;

        unsigned int  width;
        unsigned int  height;
        IMAGE_FORMAT  native_format;
        void         *pixels[IMAGE_NUM_FORMATS];

        bool LoadImageData();
};

class ImageHelper
{
    public:
        static int GetWidthLn2(Image *img)
        {
            return ImageHelper::GetPowerOf2(img->GetWidth());
        }

        static int GetHeightLn2(Image *img)
        {
            return ImageHelper::GetPowerOf2(img->GetHeight());
        }

        static int GetPowerOf2(int value)
        {
            int power = 1;
            while (1 << power != value)
            {
                ++power;
            }

            return power;
        }
};



/*  194 */
#pragma pack(push, 1)
struct RGBTexture
{
  RGBTexture();
  void Release();
  int LoadPCXFile(const char *Filename, unsigned int a3);
  unsigned int LoadFromFILE(FILE *pFile, unsigned int mode, unsigned int bCloseFile);
  int DecodePCX(char *pPcx, unsigned __int16 *pOutPixels, unsigned int uNumPixels);
  int Load(const char *pContainer, int mode);
  int Reload(const char *pContainer);

  char pName[16];
  unsigned int uNumPixels;
  unsigned __int16 uWidth;
  unsigned __int16 uHeight;
  __int16 field_18;
  __int16 field_1A;
  __int16 field_1C;
  __int16 field_1E;
  int _allocation_flags; // & 1 - malloc, else custom allocator
  unsigned __int16 *pPixels;

  struct ID3D11ShaderResourceView *d3d11_srv;
  struct D3D11_TEXTURE2D_DESC     *d3d11_desc;
};
#pragma pack(pop)



#pragma pack(push, 1)
struct Texture_MM7
{
  Texture_MM7();
  void Release();
  void *UnzipPalette();

  char pName[16];
  unsigned int uSizeOfMaxLevelOfDetail;
  unsigned int uTextureSize;
  unsigned __int16 uTextureWidth;
  unsigned __int16 uTextureHeight;
  __int16 uWidthLn2;
  __int16 uHeightLn2;
  __int16 uWidthMinus1;
  __int16 uHeightMinus1;
  short palette_id1;
  short palette_id2;
  unsigned int uDecompressedSize;
  int pBits; // 0x0002 - generate mipmaps
             // 0x0200 - 0th palette entry is transparent, else colorkey (7FF)
  unsigned __int8 *paletted_pixels;
  unsigned __int8 *pLevelOfDetail1;
  /*unsigned __int8 *pLevelOfDetail2;*/ struct ID3D11ShaderResourceView *d3d11_srv;   // replace ol SW stuff with new fields to keep data integrity
  /*unsigned __int8 *pLevelOfDetail3;*/ struct D3D11_TEXTURE2D_DESC     *d3d11_desc;
  unsigned __int16 *pPalette16;
  unsigned __int8 *pPalette24;
};
#pragma pack(pop)

















/*  323 */
enum TEXTURE_FRAME_TABLE_FLAGS
{
  TEXTURE_FRAME_TABLE_MORE_FRAMES = 0x1,
  TEXTURE_FRAME_TABLE_FIRST = 0x2,
};



/*   41 */
#pragma pack(push, 1)
struct TextureFrame
{
  char pTextureName[12];
  __int16 uTextureID;
  __int16 uAnimTime;
  __int16 uAnimLength;
  __int16 uFlags;
};
#pragma pack(pop)

/*   40 */
#pragma pack(push, 1)
struct TextureFrameTable
{
  //----- (0044D4C9) --------------------------------------------------------
  inline TextureFrameTable()
  {
    pTextures = 0;
    sNumTextures = 0;
  }
  int FromFileTxt(const char *Args);
  void ToFile();
  void FromFile(void *data_mm6, void *data_mm7, void *data_mm8);
  void LoadAnimationSequenceAndPalettes(signed int uIconID);
  int GetFrameTexture(int uFrameID, int time);
  signed int FindTextureByName(const char *Str2);


  signed int sNumTextures;
  struct TextureFrame *pTextures;
};
#pragma pack(pop)

extern struct TextureFrameTable *pTextureFrameTable;




struct OptionsMenuSkin
{
       OptionsMenuSkin();
  void Relaease();

  class Image *uTextureID_Background;      // 507C60
  class Image *uTextureID_TurnSpeed[3];    // 507C64
  class Image *uTextureID_ArrowLeft;       // 507C70
  class Image *uTextureID_ArrowRight;      // 507C74
  class Image *uTextureID_unused_0;        // 507C78
  class Image *uTextureID_unused_1;        // 507C7C
  class Image *uTextureID_unused_2;        // 507C80
  class Image *uTextureID_FlipOnExit;      // 507C84
  class Image *uTextureID_SoundLevels[10]; // 507C88
  class Image *uTextureID_AlwaysRun;       // 507CB0
  class Image *uTextureID_WalkSound;       // 507CB4
  class Image *uTextureID_ShowDamage;      // 507CB8
};
extern OptionsMenuSkin options_menu_skin; // 507C60


extern struct stru355 stru_4E82A4;// = {0x20, 0x41, 0, 0x20, 0xFF0000, 0xFF00, 0xFF, 0xFF000000};  moved to texture.h
extern struct stru355 stru_4EFCBC;// = {0x20, 0x41, 0, 0x10, 0x7C00, 0x3E0, 0x1F, 0x8000};  moved to texture.h









struct stru355
{
  int field_0;
  int field_4;
  int field_8;
  int field_C;
  int field_10;
  int field_14;
  int field_18;
  int field_1C;
};

/*  390 */
#pragma pack(push, 1)
struct stru350
{
  stru350 *_450DDE();
  bool _450DF1(const struct stru355 *p1, const struct stru355 *p2);
  unsigned int _450F55(int a2);
  int _450FB1(int a2);
  int sub_451007_scale_image_bicubic(unsigned short *pSrc, int srcWidth, int srcHeight, int srcPitch,
                                     unsigned short *pDst, int dstWidth, int dstHeight, int dstPitch,
                                     int a9, int a10);

  struct stru355 field_0;
  struct stru355 field_20;
  int field_40;
  int field_44;
  int field_48;
  int field_4C;
  int field_50;
  int field_54;
  int field_58;
  int field_5C;
};
#pragma pack(pop)

