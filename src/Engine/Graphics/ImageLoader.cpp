#include "ImageLoader.h"

#include <unordered_set>
#include <string_view>
#include <memory>

#include "Engine/Engine.h"
#include "Engine/EngineFileSystem.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/TileGenerator.h"
#include "Engine/LodTextureCache.h"
#include "Engine/LodSpriteCache.h"
#include "Engine/Graphics/PaletteManager.h"

#include "Library/Image/ImageFunctions.h"
#include "Library/Image/Pcx.h"
#include "Library/Image/Png.h"
#include "Library/LodFormats/LodImage.h"
#include "Library/LodFormats/LodSprite.h"
#include "Library/Logger/Logger.h"

// List of textures that require additional processing for transparent pixels.
// TODO(captainurist): #jsonify & move to compiled-in game data
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

static Palette MakePaletteAlpha(const Palette &palette) {
    Palette result = palette;
    result.colors[0] = Color();
    return result;
}

static Palette MakePaletteColorKey(const Palette &palette, Color key) {
    Palette result = palette;

    for (size_t i = 0; i < 256; i++)
        if (result.colors[i] == key)
            result.colors[i] = Color(); // Repeated appearances of the same color do happen, so can't break early.

    return result;
}

bool Paletted_Img_Loader::Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) {
    LodImage *tex = lod->loadTexture(resource_name);
    if (tex == nullptr)
        return false;

    // TODO(captainurist): no need to copy here.
    *indexedImage = GrayscaleImage::copy(tex->image.width(), tex->image.height(), tex->image.pixels().data());
    *palette = tex->palette;
    *rgbaImage = makeRgbaImage(*indexedImage, *palette);

    return true;
}

bool ColorKey_LOD_Loader::Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) {
    LodImage *tex = lod->loadTexture(resource_name);
    if (tex == nullptr)
        return false;

    // TODO(captainurist): no need to copy here.
    *indexedImage = GrayscaleImage::copy(tex->image.width(), tex->image.height(), tex->image.pixels().data());

    if (tex->zeroIsTransparent) {
        *palette = MakePaletteAlpha(tex->palette);
    } else {
        *palette = MakePaletteColorKey(tex->palette, colorkey);
    }

    *rgbaImage = makeRgbaImage(*indexedImage, *palette);

    return true;
}

bool Image16bit_LOD_Loader::Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) {
    LodImage *tex = lod->loadTexture(resource_name);
    if (tex == nullptr)
        return false;

    // TODO(captainurist): no need to copy here.
    *indexedImage = GrayscaleImage::copy(tex->image.width(), tex->image.height(), tex->image.pixels().data());

    if (tex->zeroIsTransparent) {
        *palette = MakePaletteAlpha(tex->palette);
    } else {
        *palette = tex->palette;
    }

    *rgbaImage = makeRgbaImage(*indexedImage, *palette);

    return true;
}

bool Alpha_LOD_Loader::Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) {
    LodImage *tex = lod->loadTexture(resource_name);
    if (tex == nullptr)
        return false;

    // TODO(captainurist): no need to copy here.
    *indexedImage = GrayscaleImage::copy(tex->image.width(), tex->image.height(), tex->image.pixels().data());
    *palette = MakePaletteAlpha(tex->palette);
    *rgbaImage = makeRgbaImage(*indexedImage, *palette);

    return true;
}

bool PCX_Loader::InternalLoad(const Blob &data, RgbaImage *rgbaImage) {
    *rgbaImage = pcx::decode(data);
    return true;
}

bool PCX_LOD_Raw_Loader::Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) {
    Blob data = lod->read(resource_name);
    if (!data) {
        logger->warning("Unable to load {}", this->resource_name);
        return false;
    }

    return InternalLoad(data, rgbaImage);
}

bool PCX_LOD_Compressed_Loader::Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) {
    Blob pcx_data = blob_func();
    if (!pcx_data) {
        logger->warning("Unable to load {}", resource_name);
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
    LodImage *tex = lod->loadTexture(this->resource_name);

    size_t w = tex->image.width();
    size_t h = tex->image.height();

    // TODO(captainurist): no need to copy here.
    *indexedImage = GrayscaleImage::copy(tex->image.width(), tex->image.height(), tex->image.pixels().data()); // NOLINT: this is not std::copy.

    // Desaturate bitmaps
    tex->palette = PaletteManager::createLoadedPalette(tex->palette);

    if (!transparentTextures.contains(this->resource_name)) {
        *palette = tex->palette;
        *rgbaImage = makeRgbaImage(*indexedImage, *palette);
    } else {
        *palette = MakePaletteAlpha(tex->palette);

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

bool Bitmaps_GEN_Loader::Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) {
    pTileGenerator->ensureTile(this->resource_name);
    *rgbaImage = png::decode(ufs->read(this->resource_name));

    // Desaturate.
    float xs = engine->config->graphics.Saturation.value();
    float xv = engine->config->graphics.Lightness.value();
    for (Color &pixel : rgbaImage->pixels())
        pixel = pixel.toHsvColorf().adjusted(0, xs, xv).toColor();

    return true;
}

bool Sprites_LOD_Loader::Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) {
    Sprite *pSprite = lod->loadSprite(this->resource_name);

    size_t w = pSprite->sprite_header->image.width();
    size_t h = pSprite->sprite_header->image.height();

    *rgbaImage = RgbaImage::solid(w, h, Color());

    for (size_t y = 0; y < h; y++) {
        for (size_t x = 0; x < w; x++) {
            uint8_t index = pSprite->sprite_header->image[y][x];
            (*rgbaImage)[y][x] = Color(index, 0, 0, index == 0 ? 0 : 255);
        }
    }

    return true;
}

