#include "ImageLoader.h"

#include <cassert>
#include <algorithm>
#include <array>
#include <functional>
#include <string_view>
#include <memory>
#include <utility>

#include "Engine/Engine.h"
#include "Engine/Resources/EngineFileSystem.h"
#include "Engine/Graphics/AtlasLayout.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/TileGenerator.h"
#include "Engine/Resources/LodTextureCache.h"
#include "Engine/Resources/LodSpriteCache.h"
#include "Engine/Graphics/PaletteManager.h"

#include "Library/Image/ImageFunctions.h"
#include "Library/Image/Pcx.h"
#include "Library/Image/Png.h"
#include "Library/LodFormats/LodFormats.h"
#include "Library/LodFormats/LodImage.h"
#include "Library/LodFormats/LodSprite.h"
#include "Library/Logger/Logger.h"

#include "Utility/Math/Float.h"

/**
 * @param image                     Image to mask.
 * @param mask                      Mask to apply.
 * @return                          For each palette entry, whether its pixels are transparent.
 */
static std::array<bool, 256> transparentPaletteEntries(const LodImage &image, const ResourceMask &mask) {
    std::array<bool, 256> result = {};
    switch (mask.mode) {
    default:
        assert(false);
        [[fallthrough]];
    case MASK_DEFAULT:
        result[0] = image.zeroIsTransparent;
        break;
    case MASK_NONE:
        break;
    case MASK_ZERO:
        result[0] = true;
        break;
    case MASK_COLOR:
        for (size_t i = 0; i < result.size(); i++)
            result[i] = image.palette.colors[i] == mask.color;
        break;
    }
    return result;
}

bool Icon_LOD_Loader::Load(RgbaImage *rgbaImage) {
    if (resource_name.ends_with(".pcx")) {
        Blob data = lod->LoadCompressedTexture(resource_name);
        if (!data) {
            MM_WARNING("Unable to load {}", resource_name);
            return false;
        }

        *rgbaImage = pcx::decode(data);
        if (mask.mode == MASK_COLOR)
            for (Color &pixel : rgbaImage->pixels())
                if (pixel == mask.color)
                    pixel = Color();
        return true;
    }

    LodImage *tex = lod->loadTexture(resource_name);
    if (tex == nullptr)
        return false;

    Palette palette = tex->palette;
    std::array<bool, 256> transparent = transparentPaletteEntries(*tex, mask);
    for (size_t i = 0; i < transparent.size(); i++)
        if (transparent[i])
            palette.colors[i] = Color();

    *rgbaImage = makeRgbaImage(tex->image, palette);
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

    AtlasLayout layout({16, 8}, tex->image.size());
    RgbaImage result = RgbaImage::uninitialized(layout.geometry().size());

    for (int i = 0; i < 126; i++) {
        Palette palette;
        palette.colors.fill(Color(0, 0, 0, 0));
        for (int index = 0; index <= 63; index++) {
            int remap = (index + i) % (2 * 63);
            if (remap >= 63)
                remap = (2 * 63) - remap;
            palette.colors[index] = tex->palette.colors[remap];
        }

        Recti cell = layout[i];
        for (int y = 0; y < cell.h; y++)
            for (int x = 0; x < cell.w; x++)
                result[y + cell.y][x + cell.x] = palette.colors[tex->image[y][x]];
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
        MM_WARNING("Unable to load {}", this->resource_name);
        return false;
    }

    return InternalLoad(data, rgbaImage);
}

static Color ProcessTransparentPixel(const GrayscaleImage &image, const Palette &palette, const std::array<bool, 256> &transparent,
                                     size_t x, size_t y) {
    size_t count = 0;
    size_t r = 0, g = 0, b = 0;

    auto processPixel = [&](size_t x, size_t y) {
        uint8_t pal = image[y][x];
        if (!transparent[pal]) {
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

    std::array<bool, 256> transparent = transparentPaletteEntries(*tex, mask);
    if (std::ranges::none_of(transparent, std::identity())) {
        *rgbaImage = makeRgbaImage(tex->image, palette);
        return true;
    }

    for (size_t i = 0; i < transparent.size(); i++)
        if (transparent[i])
            palette.colors[i] = Color();

    // Bitmaps are drawn with bilinear filtering, so transparent pixels take the color of their opaque neighbors
    // to keep the filter from bleeding the mask color into the edges.
    *rgbaImage = RgbaImage::uninitialized(w, h);
    for (size_t y = 0; y < h; y++) {
        for (size_t x = 0; x < w; x++) {
            uint8_t pal = tex->image[y][x];
            if (transparent[pal]) {
                (*rgbaImage)[y][x] = ProcessTransparentPixel(tex->image, palette, transparent, x, y);
            } else {
                (*rgbaImage)[y][x] = palette.colors[pal];
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
    if (fuzzyEquals(xs, 1.0f) && fuzzyEquals(xv, 1.0f))
        return true;

    for (Color &pixel : rgbaImage->pixels())
        pixel = pixel.toHsvColorf().adjusted(0, xs, xv).toColor();

    return true;
}

bool Sprites_LOD_Loader::Load(RgbaImage *rgbaImage) {
    LodSprite sprite = lod::decodeSprite(lod->read(this->resource_name));

    *rgbaImage = RgbaImage::uninitialized(sprite.image.width(), sprite.image.height());

    auto srcPixels = sprite.image.pixels();
    auto dstPixels = rgbaImage->pixels();
    for (size_t i = 0, size = srcPixels.size(); i < size; i++)
        dstPixels[i] = Color(srcPixels[i], 0, 0, srcPixels[i] == 0 ? 0 : 255);

    return true;
}

