#include "ImageLoader.h"

#include <unordered_set>
#include <string_view>
#include <memory>

#include "Engine/ErrorHandling.h"
#include "Engine/Graphics/IRender.h"
#include "Library/Image/PCX.h"
#include "Engine/Graphics/Sprites.h"

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

static Color *MakeImageSolid(size_t width, size_t height, uint8_t *pixels, uint8_t *palette) {
    Color *res = new Color[width * height];

    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            int index = pixels[y * width + x];
            auto r = palette[(index * 3) + 0];
            auto g = palette[(index * 3) + 1];
            auto b = palette[(index * 3) + 2];
            res[y * width + x] = Color(r, g, b);
        }
    }

    return res;
}

static Color *MakeImageAlpha(size_t width, size_t height, uint8_t *pixels, uint8_t *palette) {
    Color *res = new Color[width * height];

    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            int index = pixels[y * width + x];
            auto r = palette[(index * 3) + 0];
            auto g = palette[(index * 3) + 1];
            auto b = palette[(index * 3) + 2];
            if (index == 0) {
                res[y * width + x] = Color(0, 0, 0, 0);
            } else {
                res[y * width + x] = Color(r, g, b);
            }
        }
    }

    return res;
}

static Color *MakeImageColorKey(size_t width, size_t height, uint8_t *pixels, uint8_t *palette, Color color_key) {
    Color *res = new Color[width * height];

    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            int index = pixels[y * width + x];
            auto r = palette[(index * 3) + 0];
            auto g = palette[(index * 3) + 1];
            auto b = palette[(index * 3) + 2];
            if (Color(r, g, b) == color_key) {
                res[y * width + x] = Color();
            } else {
                res[y * width + x] = Color(r, g, b);
            }
        }
    }

    return res;
}

bool Paletted_Img_Loader::Load(size_t *out_width, size_t *out_height, Color **out_pixels,
                               uint8_t **out_palette, uint8_t **out_palettepixels) {
    *out_width = 0;
    *out_height = 0;
    *out_pixels = nullptr;
    *out_palette = nullptr;
    *out_palettepixels = nullptr;

    Texture_MM7 *tex = lod->GetTexture(lod->LoadTexture(resource_name, TEXTURE_24BIT_PALETTE));

    if ((tex == nullptr) || (tex->pPalette24 == nullptr) || (tex->paletted_pixels == nullptr)) {
        return false;
    }

    // need to copy save palette pixels
    int size = tex->header.uTextureWidth * tex->header.uTextureHeight;
    uint8_t *store = new uint8_t[size];
    memcpy(store, tex->paletted_pixels, size);
    *out_palettepixels = store;

    uint8_t *storepal = new uint8_t[3 * 256];
    memcpy(storepal, tex->pPalette24, 3 * 256);
    *out_palette = storepal;

    // make the actual image
    *out_pixels = MakeImageAlpha(tex->header.uTextureWidth,
        tex->header.uTextureHeight,
        tex->paletted_pixels, tex->pPalette24);

    if (*out_pixels == nullptr) {
        return false;
    }

    *out_width = tex->header.uTextureWidth;
    *out_height = tex->header.uTextureHeight;

    return true;
}

bool ColorKey_LOD_Loader::Load(size_t *out_width, size_t *out_height, Color **out_pixels,
                               uint8_t **out_palette, uint8_t **out_palettepixels) {
    *out_width = 0;
    *out_height = 0;
    *out_pixels = nullptr;

    Texture_MM7 *tex = lod->GetTexture(lod->LoadTexture(resource_name, TEXTURE_24BIT_PALETTE));
    if ((tex == nullptr) || (tex->pPalette24 == nullptr) ||
        (tex->paletted_pixels == nullptr)) {
        return false;
    }

    if (tex->header.pBits & 512) {
        *out_pixels = MakeImageAlpha(tex->header.uTextureWidth,
                                     tex->header.uTextureHeight,
                                     tex->paletted_pixels, tex->pPalette24);
    } else {
        *out_pixels = MakeImageColorKey(
            tex->header.uTextureWidth, tex->header.uTextureHeight,
            tex->paletted_pixels, tex->pPalette24, colorkey);
    }

    if (*out_pixels == nullptr) {
        return false;
    }

    *out_width = tex->header.uTextureWidth;
    *out_height = tex->header.uTextureHeight;
    *out_palette = tex->pPalette24;

    return true;
}

bool Image16bit_LOD_Loader::Load(size_t *out_width, size_t *out_height, Color **out_pixels,
                                 uint8_t **out_palette, uint8_t **out_palettepixels) {
    *out_width = 0;
    *out_height = 0;
    *out_pixels = nullptr;

    Texture_MM7 *tex = lod->GetTexture(lod->LoadTexture(resource_name, TEXTURE_24BIT_PALETTE));
    if ((tex == nullptr) || (tex->pPalette24 == nullptr) ||
        (tex->paletted_pixels == nullptr)) {
        return false;
    }

    if (tex->header.pBits & 512) {
        *out_pixels = MakeImageAlpha(tex->header.uTextureWidth,
                                     tex->header.uTextureHeight,
                                     tex->paletted_pixels, tex->pPalette24);
    } else {
        *out_pixels = MakeImageSolid(tex->header.uTextureWidth,
                                     tex->header.uTextureHeight,
                                     tex->paletted_pixels, tex->pPalette24);
    }

    if (*out_pixels == nullptr) {
        return false;
    }

    *out_width = tex->header.uTextureWidth;
    *out_height = tex->header.uTextureHeight;
    *out_palette = tex->pPalette24;

    return true;
}

bool Alpha_LOD_Loader::Load(size_t *out_width, size_t *out_height, Color **out_pixels,
                            uint8_t **out_palette, uint8_t **out_palettepixels) {
    *out_width = 0;
    *out_height = 0;
    *out_pixels = nullptr;

    Texture_MM7 *tex = lod->GetTexture(lod->LoadTexture(resource_name, TEXTURE_24BIT_PALETTE));
    if ((tex == nullptr) || (tex->pPalette24 == nullptr) ||
        (tex->paletted_pixels == nullptr)) {
        return false;
    }

    if ((tex->header.pBits == 0) || (tex->header.pBits & 512)) {
        *out_pixels = MakeImageAlpha(tex->header.uTextureWidth,
                                     tex->header.uTextureHeight,
                                     tex->paletted_pixels, tex->pPalette24);
    } else {
        *out_pixels = MakeImageColorKey(
            tex->header.uTextureWidth, tex->header.uTextureHeight,
            tex->paletted_pixels, tex->pPalette24, colorTable.TealMask);
    }

    if (*out_pixels == nullptr) {
        return false;
    }

    *out_width = tex->header.uTextureWidth;
    *out_height = tex->header.uTextureHeight;
    *out_palette = tex->pPalette24;

    return true;
}

bool PCX_Loader::InternalLoad(const void *file, size_t filesize,
                              size_t *width, size_t *height, Color **pixels) {
    std::unique_ptr<Color[]> result = PCX::Decode(Blob::view(file, filesize), width, height);
    if (!result)
        return false;

    *pixels = result.release();
    return true;
}

bool PCX_File_Loader::Load(size_t *width, size_t *height, Color **pixels,
                           uint8_t **out_palette, uint8_t **out_palettepixels) {
    *width = 0;
    *height = 0;
    *pixels = nullptr;
    *out_palette = nullptr;

    Blob buffer = Blob::fromFile(makeDataPath(this->resource_name));
    return InternalLoad(buffer.data(), buffer.size(), width, height, pixels);
}

bool PCX_LOD_Raw_Loader::Load(size_t *width, size_t *height, Color **pixels,
                              uint8_t **out_palette, uint8_t **out_palettepixels) {
    *width = 0;
    *height = 0;
    *pixels = nullptr;
    *out_palette = nullptr;

    Blob data = lod->LoadRaw(resource_name);
    if (!data) {
        log->warning("Unable to load {}", this->resource_name);
        return false;
    }

    return InternalLoad(data.data(), data.size(), width, height, pixels);
}

bool PCX_LOD_Compressed_Loader::Load(size_t *width, size_t *height, Color **pixels,
                                     uint8_t **out_palette, uint8_t **out_palettepixels) {
    *width = 0;
    *height = 0;
    *pixels = nullptr;
    *out_palette = nullptr;

    Blob pcx_data = lod->LoadCompressedTexture(resource_name);
    if (!pcx_data) {
        log->warning("Unable to load {}", resource_name);
        return false;
    }

    return InternalLoad(pcx_data.data(), pcx_data.size(), width, height, pixels);
}

static void ProcessTransparentPixel(const uint8_t *pixels, const uint8_t *palette,
                                    size_t x, size_t y, size_t w, size_t h, Color *rgba) {
    size_t count = 0;
    size_t r = 0, g = 0, b = 0;

    auto processPixel = [&](size_t x, size_t y) {
        int pal = pixels[y * w + x];
        if (pal != 0) {
            count++;
            b += palette[3 * pal + 2];
            g += palette[3 * pal + 1];
            r += palette[3 * pal + 0];
        }
    };

    bool canDecX = x > 0;
    bool canIncX = x < w - 1;
    bool canDecY = y > 0;
    bool canIncY = y < h - 1;

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

    *rgba = Color(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b), 0);
}

bool Bitmaps_LOD_Loader::Load(size_t *width, size_t *height, Color **out_pixels,
                              uint8_t **out_palette, uint8_t **out_palettepixels) {
    Texture_MM7 *tex = lod->GetTexture(lod->LoadTexture(this->resource_name));
    int num_pixels = tex->header.uTextureWidth * tex->header.uTextureHeight;

    Assert(tex->paletted_pixels);
    Assert(tex->pPalette24);

    Color *pixels = new Color[num_pixels];
    size_t w = tex->header.uTextureWidth;
    size_t h = tex->header.uTextureHeight;

    bool haveTransparency = transparentTextures.contains(tex->header.pName);

    for (size_t y = 0; y < h; y++) {
        for (size_t x = 0; x < w; x++) {
            size_t p = y * w + x;

            int pal = tex->paletted_pixels[p];
            if (haveTransparency && pal == 0) {
                ProcessTransparentPixel(tex->paletted_pixels, tex->pPalette24, x, y, w, h, &pixels[p]);
            } else {
                pixels[p] = Color(tex->pPalette24[3 * pal + 0], tex->pPalette24[3 * pal + 1], tex->pPalette24[3 * pal + 2]);
            }
        }
    }

    *width = tex->header.uTextureWidth;
    *height = tex->header.uTextureHeight;
    *out_pixels = pixels;
    *out_palette = tex->pPalette24;
    return true;
}

bool Sprites_LOD_Loader::Load(size_t *width, size_t *height, Color **out_pixels,
                              uint8_t **out_palette, uint8_t **out_palettepixels) {
    *width = 0;
    *height = 0;
    *out_pixels = nullptr;
    *out_palette = nullptr;

    Sprite *pSprite = lod->getSprite(this->resource_name);

    size_t w = pSprite->sprite_header->uWidth;
    size_t h = pSprite->sprite_header->uHeight;
    int numpix = w * h;

    Color *pixels = new Color[numpix];
    memset(pixels, 0, numpix * 4);

    for (size_t y = 0; y < h; y++) {
        for (size_t x = 0; x < w; x++) {
            size_t p = y * w + x;
            uint8_t bitpix = pSprite->sprite_header->bitmap[p];

            int r = 0, g = 0, b = 0, a = 0;
            r = bitpix;
            g = 0;
            b = 0;

            if (bitpix == 0) {
                a = r = g = b = 0;
            } else {
                a = 255;
            }

            pixels[p] = Color(r, g, b, a);
        }
    }

    *width = w;
    *height = h;
    *out_pixels = pixels;
    *out_palette = nullptr;
    return true;
}

