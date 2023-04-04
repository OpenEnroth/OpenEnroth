#include "ImageLoader.h"

#include <unordered_set>
#include <string_view>

#include "Engine/ErrorHandling.h"
#include "Engine/Graphics/HWLContainer.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/ImageFormatConverter.h"
#include "Engine/Graphics/PCX.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/PaletteManager.h"

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

uint32_t *MakeImageSolid(unsigned int width, unsigned int height,
                         uint8_t *pixels, uint8_t *palette) {
    uint32_t *res = new uint32_t[width * height];

    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            int index = pixels[y * width + x];
            auto r = palette[(index * 3) + 0];
            auto g = palette[(index * 3) + 1];
            auto b = palette[(index * 3) + 2];
            res[y * width + x] = color32(r, g, b);
        }
    }

    return res;
}

uint32_t *MakeImageAlpha(unsigned int width, unsigned int height,
                         uint8_t *pixels, uint8_t *palette) {
    uint32_t *res = new uint32_t[width * height];

    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            int index = pixels[y * width + x];
            auto r = palette[(index * 3) + 0];
            auto g = palette[(index * 3) + 1];
            auto b = palette[(index * 3) + 2];
            if (index == 0) {
                res[y * width + x] = color32(0, 0, 0, 0);
            } else {
                res[y * width + x] = color32(r, g, b);
            }
        }
    }

    return res;
}

uint32_t *MakeImageColorKey(unsigned int width, unsigned int height,
                            uint8_t *pixels, uint8_t *palette,
                            uint16_t color_key) {
    uint32_t *res = new uint32_t[width * height];

    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            int index = pixels[y * width + x];
            auto r = palette[(index * 3) + 0];
            auto g = palette[(index * 3) + 1];
            auto b = palette[(index * 3) + 2];
            if (color16(r, g, b) == color_key) {
                res[y * width + x] = color32(0, 0, 0, 0);
            } else {
                res[y * width + x] = color32(r, g, b);
            }
        }
    }

    return res;
}

bool Paletted_Img_Loader::Load(unsigned int *out_width,
    unsigned int *out_height, void **out_pixels,
    IMAGE_FORMAT *out_format, void **out_palette, void **out_palettepixels) {
    *out_width = 0;
    *out_height = 0;
    *out_pixels = nullptr;
    *out_format = IMAGE_INVALID_FORMAT;
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
    *out_format = IMAGE_FORMAT_R8G8B8;

    return true;
}

bool ColorKey_LOD_Loader::Load(unsigned int *out_width,
                               unsigned int *out_height, void **out_pixels,
                               IMAGE_FORMAT *out_format, void **out_palette, void **out_palettepixels) {
    *out_width = 0;
    *out_height = 0;
    *out_pixels = nullptr;
    *out_format = IMAGE_INVALID_FORMAT;

    Texture_MM7 *tex = lod->GetTexture(
        lod->LoadTexture(resource_name, TEXTURE_24BIT_PALETTE));
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
    *out_format = IMAGE_FORMAT_A8B8G8R8;
    *out_palette = tex->pPalette24;

    return true;
}

bool Image16bit_LOD_Loader::Load(unsigned int *out_width,
                                 unsigned int *out_height, void **out_pixels,
                                 IMAGE_FORMAT *out_format, void **out_palette, void **out_palettepixels) {
    *out_width = 0;
    *out_height = 0;
    *out_pixels = nullptr;
    *out_format = IMAGE_INVALID_FORMAT;

    Texture_MM7 *tex = lod->GetTexture(
        lod->LoadTexture(resource_name, TEXTURE_24BIT_PALETTE));
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
    *out_format = IMAGE_FORMAT_A8B8G8R8;
    *out_palette = tex->pPalette24;

    return true;
}

bool Alpha_LOD_Loader::Load(unsigned int *out_width, unsigned int *out_height,
                            void **out_pixels, IMAGE_FORMAT *out_format, void **out_palette, void **out_palettepixels) {
    *out_width = 0;
    *out_height = 0;
    *out_pixels = nullptr;
    *out_format = IMAGE_INVALID_FORMAT;

    Texture_MM7 *tex = lod->GetTexture(
        lod->LoadTexture(resource_name, TEXTURE_24BIT_PALETTE));
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
            tex->paletted_pixels, tex->pPalette24, colorTable.TealMask.c16());
    }

    if (*out_pixels == nullptr) {
        return false;
    }

    *out_width = tex->header.uTextureWidth;
    *out_height = tex->header.uTextureHeight;
    *out_format = IMAGE_FORMAT_A8B8G8R8;
    *out_palette = tex->pPalette24;

    return true;
}

bool PCX_Loader::InternalLoad(const void *file, size_t filesize,
                                   unsigned int *width, unsigned int *height,
                                   void **pixels, IMAGE_FORMAT *format) {
    IMAGE_FORMAT request_format = IMAGE_FORMAT_A8B8G8R8;

    *pixels = PCX::Decode(file, filesize, width, height, format, request_format);
    if (*pixels)
        return true;

    return false;
}

bool PCX_File_Loader::Load(unsigned int *width, unsigned int *height,
                           void **pixels, IMAGE_FORMAT *format, void **out_palette, void **out_palettepixels) {
    *width = 0;
    *height = 0;
    *pixels = nullptr;
    *format = IMAGE_INVALID_FORMAT;
    *out_palette = nullptr;

    Blob buffer = Blob::fromFile(MakeDataPath(this->resource_name));
    return InternalLoad(buffer.data(), buffer.size(), width, height, pixels, format);
}

bool PCX_LOD_Raw_Loader::Load(unsigned int *width, unsigned int *height,
                              void **pixels, IMAGE_FORMAT *format, void **out_palette, void **out_palettepixels) {
    *width = 0;
    *height = 0;
    *pixels = nullptr;
    *format = IMAGE_INVALID_FORMAT;
    *out_palette = nullptr;

    Blob data = lod->LoadRaw(resource_name);
    if (!data) {
        log->warning("Unable to load {}", this->resource_name);
        return false;
    }

    return InternalLoad(data.data(), data.size(), width, height, pixels, format);
}

bool PCX_LOD_Compressed_Loader::Load(unsigned int *width, unsigned int *height,
                                     void **pixels, IMAGE_FORMAT *format, void **out_palette, void **out_palettepixels) {
    *width = 0;
    *height = 0;
    *pixels = nullptr;
    *out_palette = nullptr;
    *format = IMAGE_INVALID_FORMAT;

    Blob pcx_data = lod->LoadCompressedTexture(resource_name);
    if (!pcx_data) {
        log->warning("Unable to load {}", resource_name);
        return false;
    }

    return InternalLoad(pcx_data.data(), pcx_data.size(), width, height, pixels, format);
}

static void ProcessTransparentPixel(uint8_t *pixels, uint8_t *palette,
                                    size_t x, size_t y, size_t w, size_t h, uint8_t *rgba) {
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

    rgba[0] = static_cast<uint8_t>(r);
    rgba[1] = static_cast<uint8_t>(g);
    rgba[2] = static_cast<uint8_t>(b);
    rgba[3] = 0;
}

bool Bitmaps_LOD_Loader::Load(unsigned int *width, unsigned int *height,
                              void **out_pixels, IMAGE_FORMAT *format, void **out_palette, void **out_palettepixels) {
    Texture_MM7 *tex = lod->GetTexture(lod->LoadTexture(this->resource_name));
    int num_pixels = tex->header.uTextureWidth * tex->header.uTextureHeight;

    if (!this->use_hwl) {
        Assert(tex->paletted_pixels);
        Assert(tex->pPalette24);

        uint8_t *pixels = new uint8_t[num_pixels * 4];
        size_t w = tex->header.uTextureWidth;
        size_t h = tex->header.uTextureHeight;

        bool haveTransparency = transparentTextures.contains(tex->header.pName);

        for (size_t y = 0; y < h; y++) {
            for (size_t x = 0; x < w; x++) {
                size_t p = y * w + x;

                int pal = tex->paletted_pixels[p];
                if (haveTransparency && pal == 0) {
                    ProcessTransparentPixel(tex->paletted_pixels, tex->pPalette24, x, y, w, h, &pixels[p * 4]);
                } else {
                    pixels[p * 4 + 0] = tex->pPalette24[3 * pal + 0];
                    pixels[p * 4 + 1] = tex->pPalette24[3 * pal + 1];
                    pixels[p * 4 + 2] = tex->pPalette24[3 * pal + 2];
                    pixels[p * 4 + 3] = 255;
                }
            }
        }

        *format = IMAGE_FORMAT_A8B8G8R8;
        *width = tex->header.uTextureWidth;
        *height = tex->header.uTextureHeight;
        *out_pixels = pixels;
        *out_palette = tex->pPalette24;
        return true;
    } else {
        uint16_t *pixels = new uint16_t[num_pixels];

        HWLTexture *hwl = render->LoadHwlBitmap(this->resource_name);
        if (hwl) {
            // linear scaling
            for (int s = 0; s < tex->header.uTextureHeight; ++s) {
                for (int t = 0; t < tex->header.uTextureWidth; ++t) {
                    unsigned int resampled_x = t * hwl->uWidth / tex->header.uTextureWidth,
                        resampled_y = s * hwl->uHeight / tex->header.uTextureHeight;
                    unsigned short sample = hwl->pPixels[resampled_y * hwl->uWidth + resampled_x];

                    pixels[s * tex->header.uTextureWidth + t] = sample;
                }
            }

            delete[] hwl->pPixels;
            delete hwl;
        }

        *format = IMAGE_FORMAT_A1R5G5B5;
        *width = tex->header.uTextureWidth;
        *height = tex->header.uTextureHeight;
        *out_pixels = pixels;
        return true;
    }
}

bool Sprites_LOD_Loader::Load(unsigned int *width, unsigned int *height,
                              void **out_pixels, IMAGE_FORMAT *format, void **out_palette, void **out_palettepixels) {
    *width = 0;
    *height = 0;
    *out_pixels = nullptr;
    *out_palette = nullptr;
    *format = IMAGE_INVALID_FORMAT;

    if (!this->use_hwl) {
        Sprite *pSprite = lod->GetSprite(this->resource_name);
        //Assert(thissprite->texture-> tex->paletted_pixels);
        //Assert(tex->pPalette24);

        size_t w = pSprite->sprite_header->uWidth;
        size_t h = pSprite->sprite_header->uHeight;
        int numpix = w * h;

        uint8_t *pixels = new uint8_t[numpix * 4];
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

                pixels[p * 4] = r;
                pixels[p * 4 + 1] = g;
                pixels[p * 4 + 2] = b;
                pixels[p * 4 + 3] = a;
            }
        }

        *format = IMAGE_FORMAT_A8B8G8R8;
        *width = w;
        *height = h;
        *out_pixels = pixels;
        *out_palette = nullptr;
        return true;
    } else {
        HWLTexture *hwl = render->LoadHwlSprite(this->resource_name);
        if (hwl) {
            int dst_width = hwl->uWidth;
            int dst_height = hwl->uHeight;

            int num_pixels = dst_width * dst_height;
            auto pixels = new uint16_t[num_pixels];
            if (pixels) {
                // linear scaling
                for (int s = 0; s < dst_height; ++s) {
                    for (int t = 0; t < dst_width; ++t) {
                        unsigned int resampled_x = t * hwl->uWidth / dst_width,
                            resampled_y = s * hwl->uHeight / dst_height;

                        unsigned short sample =
                            hwl->pPixels[resampled_y * hwl->uWidth + resampled_x];

                        pixels[s * dst_width + t] = sample;
                    }
                }

                delete[] hwl->pPixels;
                delete hwl;

                *width = dst_width;
                *height = dst_height;
                *format = IMAGE_FORMAT_A1R5G5B5;
            }

            *out_pixels = pixels;
            return true;
        }
    }

    return false;
}

