#pragma once

#include <string>
#include <vector>

#include "Utility/IndexedArray.h"
#include "Utility/Memory/Blob.h"

#include "Library/Binary/MemCopySerialization.h"
#include "Library/Color/Color.h"

class ImageLoader;
class Image {
 public:
    explicit Image(bool lazy_initialization = true): lazy_initialization(lazy_initialization) {}
    virtual ~Image() {}

    static Image *Create(unsigned int width, unsigned int height, const Color *pixels = nullptr);
    static Image *Create(ImageLoader *loader);

    int GetWidth();
    int GetHeight();
    const Color *GetPixels();

    /**
     * @return                              Returns pointer to image R8G8B8 palette. Size 3 * 256.
     */
    const uint8_t *GetPalette();

    /**
     * @return                              Returns pointer to image pixels 8 bit palette lookup. Size 1 * width * height.
     */
    const uint8_t *GetPalettePixels();

    std::string *GetName();

    bool Release();

 protected:
    bool lazy_initialization = false;
    bool initialized = false;
    ImageLoader *loader = nullptr;

    size_t width = 0;
    size_t height = 0;
    Color *pixels = nullptr;
    uint8_t *palette24 = nullptr;
    uint8_t *palettepixels = nullptr;

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
MM_DECLARE_MEMCOPY_SERIALIZABLE(TextureHeader)

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
     /**
     * @param   frameID        TextureFrameTable index
     * @return                 Total length of texture animation 
     */
    int textureFrameAnimLength(int frameID);
    /**
    * @param   frameID        TextureFrameTable index
    * @return                 Dwell time of this texture
    */
    int textureFrameAnimTime(int frameID);
    int64_t FindTextureByName(const std::string &Str2);

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
