#include "ImageLoader.h"

#include <unordered_set>
#include <string_view>
#include <memory>
#include <utility>

#include "Engine/Engine.h"
#include "Engine/Resources/EngineFileSystem.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/TileGenerator.h"
#include "Engine/Resources/LodTextureCache.h"
#include "Engine/Resources/LodSpriteCache.h"
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

bool Paletted_Img_Loader::Load(RgbaImage *rgbaImage) {
    LodImage *tex = lod->loadTexture(resource_name);
    if (tex == nullptr)
        return false;

    *rgbaImage = makeRgbaImage(tex->image, tex->palette);

    return true;
}

bool ColorKey_LOD_Loader::Load(RgbaImage *rgbaImage) {
    LodImage *tex = lod->loadTexture(resource_name);
    if (tex == nullptr)
        return false;

    Palette palette;
    if (tex->zeroIsTransparent) {
        palette = MakePaletteAlpha(tex->palette);
    } else {
        palette = MakePaletteColorKey(tex->palette, colorkey);
    }

    *rgbaImage = makeRgbaImage(tex->image, palette);

    return true;
}

bool Image16bit_LOD_Loader::Load(RgbaImage *rgbaImage) {
    LodImage *tex = lod->loadTexture(resource_name);
    if (tex == nullptr)
        return false;

    Palette palette;
    if (tex->zeroIsTransparent) {
        palette = MakePaletteAlpha(tex->palette);
    } else {
        palette = tex->palette;
    }

    *rgbaImage = makeRgbaImage(tex->image, palette);

    return true;
}

bool Alpha_LOD_Loader::Load(RgbaImage *rgbaImage) {
    LodImage *tex = lod->loadTexture(resource_name);
    if (tex == nullptr)
        return false;

    *rgbaImage = makeRgbaImage(tex->image, MakePaletteAlpha(tex->palette));

    return true;
}

bool Buff_LOD_Loader::Load(RgbaImage *rgbaImage) {
    LodImage *tex = lod->loadTexture(resource_name);
    if (tex == nullptr)
        return false;

    // So, the way this works.
    //
    // Icons are made using a gradient of 64 different colors. E.g. for a feather (feather fall spell) the
    // gradient literally goes along the length of the feather. The border of the icon is single color.
    //
    // Then, we have a palette that has a colored gradient that's 64 colors long, from lighter to darker colors.
    // We extend this gradient by transposing it and adding it to itself, so we get a gradient 126 colors long.
    // Why 126 colors? Because we don't duplicate the starting & ending colors. The nice thing about the
    // resulting gradient is that it's looped.
    //
    // What we do next can be visualized like this:
    //
    // Color gradient: |-------------------------------------------------------------------------------|
    // Palette:                      |-------------------------------------|
    //
    // We just slide the palette mapping along the gradient, wrapping around as necessary. This results in a nice
    // animation.
    //
    // This used to be done on draw, we're just generating a texture atlas. Alternative is to do this in-shader,
    // but generating an atlas is easier to do.

    RgbaImage result = RgbaImage::uninitialized(tex->image.width() * 16, tex->image.height() * 8);

    for (int i = 0; i < 126; i++) {
        Palette palette;
        palette.colors.fill(Color(0, 0, 0, 0));
        for (int index = 0; index <= 63; index++) {
            int remap = (index + i) % (2 * 63);
            if (remap >= 63)
                remap = (2 * 63) - remap;
            palette.colors[index] = tex->palette.colors[remap];
        }

        int dx = (i % 16) * tex->image.width();
        int dy = (i / 16) * tex->image.height();
        for (int y = 0, maxy = tex->image.height(); y < maxy; y++)
            for (int x = 0, maxx = tex->image.width(); x < maxx; x++)
                result[y + dy][x + dx] = palette.colors[tex->image[y][x]];
    }

    *rgbaImage = std::move(result);

    return true;
}

bool PCX_Loader::InternalLoad(const Blob &data, RgbaImage *rgbaImage) {
    *rgbaImage = pcx::decode(data);
    return true;
}

bool PCX_LOD_Raw_Loader::Load(RgbaImage *rgbaImage) {
    Blob data = lod->read(resource_name);
    if (!data) {
        logger->warning("Unable to load {}", this->resource_name);
        return false;
    }

    return InternalLoad(data, rgbaImage);
}

bool PCX_LOD_Compressed_Loader::Load(RgbaImage *rgbaImage) {
    Blob pcx_data = blob_func();
    if (!pcx_data) {
        logger->warning("Unable to load {}", resource_name);
        return false;
    }

    bool result = InternalLoad(pcx_data, rgbaImage);

    for (Color &pixel : rgbaImage->pixels())
        if (pixel == colorkey)
            pixel = Color();

    return result;
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

bool Bitmaps_LOD_Loader::Load(RgbaImage *rgbaImage) {
    LodImage *tex = lod->loadTexture(this->resource_name);

    size_t w = tex->image.width();
    size_t h = tex->image.height();

    // Desaturate bitmaps
    Palette palette = PaletteManager::createLoadedPalette(tex->palette);

    if (!transparentTextures.contains(this->resource_name)) {
        *rgbaImage = makeRgbaImage(tex->image, palette);
    } else {
        palette = MakePaletteAlpha(palette);

        *rgbaImage = RgbaImage::uninitialized(w, h);
        for (size_t y = 0; y < h; y++) {
            for (size_t x = 0; x < w; x++) {
                uint8_t pal = tex->image[y][x];
                if (pal == 0) {
                    (*rgbaImage)[y][x] = ProcessTransparentPixel(tex->image, palette, x, y);
                } else {
                    (*rgbaImage)[y][x] = palette.colors[pal];
                }
            }
        }
    }

    return true;
}

bool Bitmaps_GEN_Loader::Load(RgbaImage *rgbaImage) {
    pTileGenerator->ensureTile(this->resource_name);
    *rgbaImage = png::decode(ufs->read(this->resource_name));

    // Desaturate.
    float xs = engine->config->graphics.Saturation.value();
    float xv = engine->config->graphics.Lightness.value();
    for (Color &pixel : rgbaImage->pixels())
        pixel = pixel.toHsvColorf().adjusted(0, xs, xv).toColor();

    return true;
}

bool Sprites_LOD_Loader::Load(RgbaImage *rgbaImage) {
    Sprite *pSprite = lod->loadSprite(this->resource_name);

    size_t w = pSprite->sprite_header->image.width();
    size_t h = pSprite->sprite_header->image.height();

    *rgbaImage = RgbaImage::solid(Color(), w, h);

    for (size_t y = 0; y < h; y++) {
        for (size_t x = 0; x < w; x++) {
            uint8_t index = pSprite->sprite_header->image[y][x];
            (*rgbaImage)[y][x] = Color(index, 0, 0, index == 0 ? 0 : 255);
        }
    }

    return true;
}

