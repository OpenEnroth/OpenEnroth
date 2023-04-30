#pragma once

#include <string>
#include <vector>

#include "Utility/IndexedArray.h"
#include "Utility/Memory/Blob.h"

enum class IMAGE_FORMAT {
    IMAGE_FORMAT_R5G6B5 = 0,
    IMAGE_FORMAT_A1R5G5B5 = 1,
    IMAGE_FORMAT_A8R8G8B8 = 2,
    IMAGE_FORMAT_R8G8B8 = 3,
    IMAGE_FORMAT_R8G8B8A8 = 4,
    IMAGE_FORMAT_A8B8G8R8 = 5,

    IMAGE_NUM_FORMATS = 6,
    IMAGE_INVALID_FORMAT = -1,
};
using enum IMAGE_FORMAT;

unsigned int IMAGE_FORMAT_BytesPerPixel(IMAGE_FORMAT format);

class ImageLoader;
class Image {
 public:
    explicit Image(bool lazy_initialization = true): lazy_initialization(lazy_initialization) {}
    virtual ~Image() {}

    static Image *Create(unsigned int width, unsigned int height,
                         IMAGE_FORMAT format, const void *pixels = nullptr);
    static Image *Create(ImageLoader *loader);

    int GetWidth();
    int GetHeight();
    const void *GetPixels(IMAGE_FORMAT format);
    const void *GetPalette();
    const void *GetPalettePixels();
    IMAGE_FORMAT GetFormat() const { return native_format; }

    std::string *GetName();

    bool Release();

 protected:
    bool lazy_initialization = false;
    bool initialized = false;
    ImageLoader *loader = nullptr;

    unsigned int width = 0;
    unsigned int height = 0;
    IMAGE_FORMAT native_format = IMAGE_INVALID_FORMAT;
    IndexedArray<void *, IMAGE_NUM_FORMATS> pixels = {{}};
    void *palette24 = nullptr;
    void *palettepixels = nullptr;

    virtual bool LoadImageData();
};

class ImageHelper {
 public:
    static int GetWidthLn2(Image *img) {
        return ImageHelper::GetPowerOf2(img->GetWidth());
    }

    static int GetHeightLn2(Image *img) {
        return ImageHelper::GetPowerOf2(img->GetHeight());
    }

    static int GetPowerOf2(int value) {
        int power = 1;
        while (1 << power != value) {
            ++power;
        }

        return power;
    }
};

#pragma pack(push, 1)
struct TextureHeader {
    char pName[16];
    uint32_t uSizeOfMaxLevelOfDetail;
    uint32_t uTextureSize;
    uint16_t uTextureWidth;
    uint16_t uTextureHeight;
    int16_t uWidthLn2;  // log2(uTextureWidth)
    int16_t uHeightLn2;  // log2(uTextureHeight)
    int16_t uWidthMinus1;
    int16_t uHeightMinus1;
    int16_t palette_id1;
    int16_t palette_id2;
    uint32_t uDecompressedSize;
    uint32_t pBits;  // 0x0002 - generate mipmaps
                     // 0x0200 - 0th palette entry is transparent, else colorkey
                     // 0x0400 - don't free buffers (???)
};
#pragma pack(pop)

struct Texture_MM7 {
    Texture_MM7();
    void Release();

    TextureHeader header;
    uint8_t *paletted_pixels;
    uint8_t *pLevelOfDetail1;
    uint8_t *pPalette24;
};

/*  323 */
// TODO(captainurist): where is this used?
enum TEXTURE_FRAME_TABLE_FLAGS {
    TEXTURE_FRAME_TABLE_MORE_FRAMES = 0x1,
    TEXTURE_FRAME_TABLE_FIRST = 0x2,
};

class Texture;
class TextureFrame {
 public:
    inline TextureFrame() : tex(nullptr) {}

    std::string name = "null";
    int16_t uAnimTime = 0;
    int16_t uAnimLength = 0;
    int16_t uFlags = 0;  // 1 for anim

    Texture *GetTexture();

 protected:
    Texture *tex;
};

struct TextureFrameTable {
    void FromFile(const Blob &data_mm6, const Blob &data_mm7, const Blob &data_mm8);
    void LoadAnimationSequenceAndPalettes(int uIconID);
    Texture *GetFrameTexture(int frameId, int time);
    int64_t FindTextureByName(const char *Str2);

    std::vector<TextureFrame> textures;
};

extern TextureFrameTable *pTextureFrameTable;

struct OptionsMenuSkin {
    OptionsMenuSkin();
    void Release();

    Image *uTextureID_Background;       // 507C60
    Image *uTextureID_TurnSpeed[3];     // 507C64
    Image *uTextureID_ArrowLeft;        // 507C70
    Image *uTextureID_ArrowRight;       // 507C74
    Image *uTextureID_unused_0;         // 507C78
    Image *uTextureID_unused_1;         // 507C7C
    Image *uTextureID_unused_2;         // 507C80
    Image *uTextureID_FlipOnExit;       // 507C84
    Image *uTextureID_SoundLevels[10];  // 507C88
    Image *uTextureID_AlwaysRun;        // 507CB0
    Image *uTextureID_WalkSound;        // 507CB4
    Image *uTextureID_ShowDamage;       // 507CB8
};
extern OptionsMenuSkin options_menu_skin;  // 507C60

extern struct stru355 stru_4E82A4;  // = {0x20, 0x41, 0, 0x20, 0xFF0000, 0xFF00,
                                    // 0xFF, 0xFF000000};  moved to texture.h
extern struct stru355 stru_4EFCBC;  // = {0x20, 0x41, 0, 0x10, 0x7C00, 0x3E0,
                                    // 0x1F, 0x8000};  moved to texture.h

struct stru355 {
    int field_0;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
    int field_14;
    int field_18;
    int field_1C;
};
