#include "Engine/Log.h"

#include "Engine/ZlibWrapper.h"

#include "Engine/Graphics/HWLContainer.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/ImageFormatConverter.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/PCX.h"
#include "Engine/Graphics/Sprites.h"

#include "Platform/Api.h"


uint32_t *MakeImageSolid(unsigned int width, unsigned int height,
                         uint8_t *pixels, uint8_t *palette) {
    uint32_t *res = new uint32_t[width * height];

    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            int index = pixels[y * width + x];
            auto r = palette[(index * 3) + 0];
            auto g = palette[(index * 3) + 1];
            auto b = palette[(index * 3) + 2];
            res[y * width + x] = Color32(r, g, b);
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
                res[y * width + x] = 0x00000000;
            } else {
                res[y * width + x] = Color32(r, g, b);
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
            if (Color16(r, g, b) == color_key) {
                res[y * width + x] = 0x00000000;
            } else {
                res[y * width + x] = Color32(r, g, b);
            }
        }
    }

    return res;
}

bool ColorKey_LOD_Loader::Load(unsigned int *out_width,
                               unsigned int *out_height, void **out_pixels,
                               IMAGE_FORMAT *out_format) {
    *out_width = 0;
    *out_height = 0;
    *out_pixels = nullptr;
    *out_format = IMAGE_INVALID_FORMAT;

    Texture_MM7 *tex = lod->GetTexture(
        lod->LoadTexture(resource_name.c_str(), TEXTURE_24BIT_PALETTE));
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
    *out_format = IMAGE_FORMAT_A8R8G8B8;

    return true;
}

bool Image16bit_LOD_Loader::Load(unsigned int *out_width,
                                 unsigned int *out_height, void **out_pixels,
                                 IMAGE_FORMAT *out_format) {
    *out_width = 0;
    *out_height = 0;
    *out_pixels = nullptr;
    *out_format = IMAGE_INVALID_FORMAT;

    Texture_MM7 *tex = lod->GetTexture(
        lod->LoadTexture(resource_name.c_str(), TEXTURE_24BIT_PALETTE));
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
    *out_format = IMAGE_FORMAT_A8R8G8B8;

    return true;
}

bool Alpha_LOD_Loader::Load(unsigned int *out_width, unsigned int *out_height,
                            void **out_pixels, IMAGE_FORMAT *out_format) {
    *out_width = 0;
    *out_height = 0;
    *out_pixels = nullptr;
    *out_format = IMAGE_INVALID_FORMAT;

    Texture_MM7 *tex = lod->GetTexture(
        lod->LoadTexture(resource_name.c_str(), TEXTURE_24BIT_PALETTE));
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
            tex->paletted_pixels, tex->pPalette24, 0x7FF);
    }

    if (*out_pixels == nullptr) {
        return false;
    }

    *out_width = tex->header.uTextureWidth;
    *out_height = tex->header.uTextureHeight;
    *out_format = IMAGE_FORMAT_A8R8G8B8;

    return true;
}

bool PCX_Loader::DecodePCX(const void *pcx_data, uint16_t *pOutPixels,
                           unsigned int *width, unsigned int *height) {
    return PCX::Decode(pcx_data, pOutPixels, width, height);
}

bool PCX_File_Loader::Load(unsigned int *width, unsigned int *height,
                           void **pixels, IMAGE_FORMAT *format) {
    *width = 0;
    *height = 0;
    *pixels = nullptr;
    *format = IMAGE_INVALID_FORMAT;

    FILE *file = fcaseopen(this->resource_name.c_str(), "rb");
    if (!file) {
        log->Warning("Unable to load %s", this->resource_name.c_str());
        return false;
    }

    fseek(file, 0, SEEK_END);
    size_t filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    bool res = InternalLoad(file, filesize, width, height, pixels, format);

    fclose(file);

    return res;
}

bool PCX_File_Loader::InternalLoad(void *file, size_t filesize,
                                   unsigned int *width, unsigned int *height,
                                   void **pixels, IMAGE_FORMAT *format) {
    if (!PCX::IsValid(file)) {
        return false;
    }

    PCX::GetSize(file, width, height);
    unsigned int num_pixels = *width * *height;
    *pixels = new uint16_t[num_pixels + 2];

    if (pixels) {
        if (!this->DecodePCX(file, (uint16_t*)*pixels, width,
                             height)) {
            delete[] *(uint16_t**)pixels;
            *pixels = nullptr;
        } else {
            *format = IMAGE_FORMAT_R5G6B5;
        }
    }

    return *pixels != nullptr;
}

bool PCX_LOD_File_Loader::Load(unsigned int *width, unsigned int *height,
                               void **pixels, IMAGE_FORMAT *format) {
    *width = 0;
    *height = 0;
    *pixels = nullptr;
    *format = IMAGE_INVALID_FORMAT;

    size_t size;
    void *data = lod->LoadRaw(resource_name, &size);
    if (data == nullptr) {
        log->Warning("Unable to load %s", this->resource_name.c_str());
        return false;
    }

    bool res = InternalLoad(data, size, width, height, pixels, format);

    free(data);

    return res;
}

bool PCX_LOD_Loader::Load(unsigned int *width, unsigned int *height,
                          void **pixels, IMAGE_FORMAT *format) {
    *width = 0;
    *height = 0;
    *pixels = nullptr;
    *format = IMAGE_INVALID_FORMAT;

    size_t data_size = 0;
    void *pcx_data = lod->LoadCompressedTexture(resource_name, &data_size);
    if (pcx_data == nullptr) {
        log->Warning("Unable to load %s", resource_name.c_str());
        return false;
    }

    if (!PCX::IsValid(pcx_data)) {
        free(pcx_data);
        return false;
    }

    PCX::GetSize(pcx_data, width, height);
    unsigned int num_pixels = *width * *height;
    *pixels = new uint16_t[num_pixels + 2];

    if (pixels) {
        if (!this->DecodePCX(pcx_data, (uint16_t*)*pixels, width,
                             height)) {
            delete[] *(uint16_t**)pixels;
            *pixels = nullptr;
        } else {
            *format = IMAGE_FORMAT_R5G6B5;
        }
    }

    free(pcx_data);

    return *pixels != nullptr;
}

bool Bitmaps_LOD_Loader::Load(unsigned int *width, unsigned int *height,
                              void **out_pixels, IMAGE_FORMAT *format) {
    *width = 0;
    *height = 0;
    *out_pixels = nullptr;
    *format = IMAGE_INVALID_FORMAT;

    auto tex = lod->GetTexture(lod->LoadTexture(this->resource_name.c_str()));

    int num_pixels = tex->header.uTextureWidth * tex->header.uTextureHeight;
    auto pixels = new uint16_t[num_pixels];
    if (pixels) {
        *width = tex->header.uTextureWidth;
        *height = tex->header.uTextureHeight;
        *format = IMAGE_FORMAT_A1R5G5B5;

        if (tex->header.pBits & 2) {  // hardware bitmap
            HWLTexture *hwl = render->LoadHwlBitmap(this->resource_name.c_str());
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

            *out_pixels = pixels;
            return true;
        }
    }

    return false;
}

bool Sprites_LOD_Loader::Load(unsigned int *width, unsigned int *height,
                              void **out_pixels, IMAGE_FORMAT *format) {
    *width = 0;
    *height = 0;
    *out_pixels = nullptr;
    *format = IMAGE_INVALID_FORMAT;

    HWLTexture *hwl = render->LoadHwlSprite(this->resource_name.c_str());
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

    return false;
}
