#include "ImageLoader.h"

#include <unordered_set>
#include <string_view>
#include <memory>

#include "Engine/ErrorHandling.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Texture_MM7.h"
#include "Engine/Snapshots/CommonSnapshots.h"
#include "Engine/LodTextureCache.h"
#include "Engine/LodSpriteCache.h"
#include "Engine/LOD.h"

#include "Library/Image/ImageFunctions.h"
#include "Library/Image/PCX.h"
#include "Library/Logger/Logger.h"

#include "Utility/DataPath.h"

// List of textures that require additional processing for transparent pixels.
// TODO: move to OpenEnroth config file.
static const std::unordered_set<std::string_view> transparentTextures = {
    "hwtrdre",
    "hwtrdrne",
    "hwtrdrs",
    "hwtrdrsw",
    "hwtrdrxne",
    "hwtrdrxse",
    "hwtrdrn",
    "hwtrdrnw",
    "hwtrdrse",
    "hwtrdrw",
    "hwtrdrxnw",
    "hwtrdrxsw"
};

static Palette MakePaletteSolid(uint8_t *palette24) {
    Palette result;
    for (size_t i = 0; i < 256; i++)
        result.colors[i] = Color(palette24[i * 3], palette24[i * 3 + 1], palette24[i * 3 + 2]);
    return result;
}

static Palette MakePaletteAlpha(uint8_t *palette24) {
    Palette result = MakePaletteSolid(palette24);
    result.colors[0] = Color();
    return result;
}

static Palette MakePaletteColorKey(uint8_t *palette24, Color key) {
    Palette result = MakePaletteSolid(palette24);

    for (size_t i = 0; i < 256; i++)
        if (result.colors[i] == key)
            result.colors[i] = Color(); // Repeated appearances of the same color do happen, so can't break early.

    return result;
}

ImageLoader::ImageLoader() {
    this->log = EngineIocContainer::ResolveLogger();
}

bool Paletted_Img_Loader::Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) {
    Texture_MM7 *tex = lod->loadTexture(resource_name);
    if ((tex == nullptr) || (tex->pPalette24 == nullptr) || (tex->paletted_pixels == nullptr))
        return false;

    *indexedImage = GrayscaleImage::copy(tex->header.uTextureWidth, tex->header.uTextureHeight, tex->paletted_pixels);
    *palette = MakePaletteSolid(tex->pPalette24);
    *rgbaImage = makeRgbaImage(*indexedImage, *palette);

    return true;
}

bool ColorKey_LOD_Loader::Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) {
    Texture_MM7 *tex = lod->loadTexture(resource_name);
    if ((tex == nullptr) || (tex->pPalette24 == nullptr) || (tex->paletted_pixels == nullptr))
        return false;

    *indexedImage = GrayscaleImage::copy(tex->header.uTextureWidth, tex->header.uTextureHeight, tex->paletted_pixels);

    if (tex->header.pBits & 512) {
        *palette = MakePaletteAlpha(tex->pPalette24);
    } else {
        *palette = MakePaletteColorKey(tex->pPalette24, colorkey);
    }

    *rgbaImage = makeRgbaImage(*indexedImage, *palette);

    return true;
}

bool Image16bit_LOD_Loader::Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) {
    Texture_MM7 *tex = lod->loadTexture(resource_name);
    if ((tex == nullptr) || (tex->pPalette24 == nullptr) || (tex->paletted_pixels == nullptr))
        return false;

    *indexedImage = GrayscaleImage::copy(tex->header.uTextureWidth, tex->header.uTextureHeight, tex->paletted_pixels);

    if (tex->header.pBits & 512) {
        *palette = MakePaletteAlpha(tex->pPalette24);
    } else {
        *palette = MakePaletteSolid(tex->pPalette24);
    }

    *rgbaImage = makeRgbaImage(*indexedImage, *palette);

    return true;
}

bool Alpha_LOD_Loader::Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) {
    Texture_MM7 *tex = lod->loadTexture(resource_name);
    if ((tex == nullptr) || (tex->pPalette24 == nullptr) || (tex->paletted_pixels == nullptr))
        return false;

    *indexedImage = GrayscaleImage::copy(tex->header.uTextureWidth, tex->header.uTextureHeight, tex->paletted_pixels);

    if ((tex->header.pBits == 0) || (tex->header.pBits & 512)) {
        *palette = MakePaletteAlpha(tex->pPalette24);
    } else {
        *palette = MakePaletteColorKey(tex->pPalette24, colorTable.TealMask);
    }

    *rgbaImage = makeRgbaImage(*indexedImage, *palette);

    return true;
}

bool PCX_Loader::InternalLoad(const Blob &data, RgbaImage *rgbaImage) {
    *rgbaImage = PCX::Decode(data);
    return true;
}

bool PCX_File_Loader::Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) {
    Blob buffer = Blob::fromFile(makeDataPath(this->resource_name));
    return InternalLoad(buffer, rgbaImage);
}

bool PCX_LOD_Raw_Loader::Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) {
    Blob data = lod->LoadRaw(resource_name);
    if (!data) {
        log->warning("Unable to load {}", this->resource_name);
        return false;
    }

    return InternalLoad(data, rgbaImage);
}

bool PCX_LOD_Compressed_Loader::Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) {
    Blob pcx_data = blob_func();
    if (!pcx_data) {
        log->warning("Unable to load {}", resource_name);
        return false;
    }

    return InternalLoad(pcx_data, rgbaImage);
}

static Color ProcessTransparentPixel(const GrayscaleImage &image, const Palette &palette, size_t x, size_t y) {
    size_t count = 0;
    size_t r = 0, g = 0, b = 0;

    auto processPixel = [&](size_t x, size_t y) {
        uint8_t pal = image[y][x];
        if (pal != 0) {
            count++;
            r += palette.colors[pal].r;
            g += palette.colors[pal].g;
            b += palette.colors[pal].b;
        }
    };

    bool canDecX = x > 0;
    bool canIncX = x < image.width() - 1;
    bool canDecY = y > 0;
    bool canIncY = y < image.height() - 1;

    if (canDecX && canDecY)
        processPixel(x - 1, y - 1);
    if (canDecX)
        processPixel(x - 1, y);
    if (canDecX && canIncY)
        processPixel(x - 1, y + 1);
    if (canDecY)
        processPixel(x, y - 1);
    if (canIncY)
        processPixel(x, y + 1);
    if (canIncX && canDecY)
        processPixel(x + 1, y - 1);
    if (canIncX)
        processPixel(x + 1, y);
    if (canIncX && canIncY)
        processPixel(x + 1, y + 1);

    if (count != 0) {
        r /= count;
        g /= count;
        b /= count;
    }

    return Color(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b), 0);
}

bool Bitmaps_LOD_Loader::Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) {
    Texture_MM7 *tex = lod->loadTexture(this->resource_name);
    int num_pixels = tex->header.uTextureWidth * tex->header.uTextureHeight;

    Assert(tex->paletted_pixels);
    Assert(tex->pPalette24);

    std::string name;
    reconstruct(tex->header.pName, &name);

    size_t w = tex->header.uTextureWidth;
    size_t h = tex->header.uTextureHeight;

    *indexedImage = GrayscaleImage::copy(w, h, tex->paletted_pixels); // NOLINT: this is not std::copy.

    if (!transparentTextures.contains(name)) {
        *palette = MakePaletteSolid(tex->pPalette24);
        *rgbaImage = makeRgbaImage(*indexedImage, *palette);
    } else {
        *palette = MakePaletteAlpha(tex->pPalette24);

        *rgbaImage = RgbaImage::uninitialized(w, h);
        for (size_t y = 0; y < h; y++) {
            for (size_t x = 0; x < w; x++) {
                uint8_t pal = (*indexedImage)[y][x];
                if (pal == 0) {
                    (*rgbaImage)[y][x] = ProcessTransparentPixel(*indexedImage, *palette, x, y);
                } else {
                    (*rgbaImage)[y][x] = palette->colors[pal];
                }
            }
        }
    }

    return true;
}

bool Sprites_LOD_Loader::Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) {
    Sprite *pSprite = lod->loadSprite(this->resource_name);

    size_t w = pSprite->sprite_header->uWidth;
    size_t h = pSprite->sprite_header->uHeight;

    *rgbaImage = RgbaImage::solid(w, h, Color());

    for (size_t y = 0; y < h; y++) {
        for (size_t x = 0; x < w; x++) {
            uint8_t index = pSprite->sprite_header->bitmap[y][x];
            (*rgbaImage)[y][x] = Color(index, 0, 0, index == 0 ? 0 : 255);
        }
    }

    return true;
}

