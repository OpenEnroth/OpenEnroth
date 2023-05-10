#include "Engine/Graphics/Image.h"

#include <algorithm>

#include "Engine/Engine.h"

#include "Engine/Graphics/ImageFormatConverter.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Texture.h"

#include "Engine/Serialization/LegacyImages.h"

#include "Library/Serialization/EnumSerialization.h"


struct TextureFrameTable *pTextureFrameTable;

stru355 stru_4E82A4 = {0x20, 0x41, 0, 0x20, 0xFF0000, 0xFF00, 0xFF, static_cast<int>(0xFF000000)};
stru355 stru_4EFCBC = {0x20, 0x41, 0, 0x10, 0x7C00, 0x3E0, 0x1F, 0x8000};

// Texture_MM7 pTex_F7CE30;

MM_DEFINE_ENUM_MAGIC_SERIALIZATION_FUNCTIONS(IMAGE_FORMAT)

unsigned int IMAGE_FORMAT_BytesPerPixel(IMAGE_FORMAT format) {
    switch (format) {
        case IMAGE_FORMAT_R5G6B5:
            return 2;
        case IMAGE_FORMAT_A1R5G5B5:
            return 2;
        case IMAGE_FORMAT_A8R8G8B8:
            return 4;
        case IMAGE_FORMAT_R8G8B8:
            return 3;
        case IMAGE_FORMAT_R8G8B8A8:
            return 4;
        case IMAGE_FORMAT_A8B8G8R8:
            return 4;

        default:
            Error("Invalid format: %d", format);
            return 0;
    }
}

Texture *TextureFrame::GetTexture() {
    if (!this->tex) {
        this->tex = assets->GetBitmap(this->name);
    }
    return this->tex;
}

void TextureFrameTable::FromFile(const Blob &data_mm6, const Blob &data_mm7, const Blob &data_mm8) {
    (void) data_mm6;
    (void) data_mm8;

    deserialize(data_mm7, appendVia<TextureFrame_MM7>(&textures));

    assert(!textures.empty());
}

void TextureFrameTable::LoadAnimationSequenceAndPalettes(int uFrameID) {
    if (uFrameID <= textures.size() && uFrameID >= 0) {
        for (unsigned int i = uFrameID;; ++i) {
            // this->pTextures[i].uTextureID =
            // pBitmaps_LOD->LoadTexture(this->pTextures[i].pTextureName,
            // TEXTURE_DEFAULT);

            // if (this->pTextures[i].uTextureID != -1)
            //    pBitmaps_LOD->pTextures[this->pTextures[i].uTextureID].palette_id2
            //    =
            //    pPaletteManager->LoadPalette(pBitmaps_LOD->pTextures[this->pTextures[i].uTextureID].palette_id1);

            if (textures[i].uFlags & 1) break;
        }
    }
    return;
}

int64_t TextureFrameTable::FindTextureByName(const std::string &Str2) {
    std::string name = toLower(Str2);

    for (size_t i = 0; i < textures.size(); ++i)
        if (textures[i].name == name)
            return i;
    return -1;
}

Texture *TextureFrameTable::GetFrameTexture(int frameId, int time) {
    int animLength = textures[frameId].uAnimLength;

    if (textures[frameId].uFlags & 1 && animLength != 0) {
        int step = (time >> 3) % animLength;
        while (textures[frameId].uAnimTime < step) {
            step -= textures[frameId].uAnimTime;
            ++frameId;
        }
    }

    return textures[frameId].GetTexture();
}

void Texture_MM7::Release() {
    header.pName[0] = 0;

    if (header.pBits & 0x0400) {
        __debugbreak();
    }

    if (paletted_pixels != nullptr) {
        free(paletted_pixels);
        paletted_pixels = nullptr;
    }

    if (pPalette24 != nullptr) {
        free(pPalette24);
        pPalette24 = nullptr;
    }

    pLevelOfDetail1 = nullptr;

    memset(&header, 0, sizeof(header));
}

Texture_MM7::Texture_MM7() {
    memset(&header, 0, sizeof(header));
    paletted_pixels = nullptr;
    pLevelOfDetail1 = nullptr;
    pPalette24 = nullptr;
}

Image *Image::Create(ImageLoader *loader) {
    Image *img = new Image();
    if (img) {
        img->loader = loader;
    }

    return img;
}

bool Image::LoadImageData() {
    if (!initialized) {
        void *data = nullptr;
        void *palette = nullptr;
        initialized = loader->Load(&width, &height, &data, &native_format, &palette);
        if (initialized && native_format != IMAGE_INVALID_FORMAT) {
            pixels[native_format] = data;
            palette24 = palette;
        }
    }

    if ((width == 0 || height == 0) && initialized) __debugbreak();

    return initialized;
}

int Image::GetWidth() {
    if (!initialized) {
        LoadImageData();
    }

    if (initialized) {
        if (width == 0) __debugbreak();
        return width;
    }

    return 0;
}

int Image::GetHeight() {
    if (!initialized) {
        LoadImageData();
    }

    if (initialized) {
        if (height == 0) __debugbreak();
        return height;
    }

    return 0;
}

Image *Image::Create(unsigned int width, unsigned int height,
                     IMAGE_FORMAT format, const void *pixels) {
    if (width == 0 || height == 0) __debugbreak();

    Image *img = new Image(false);
    if (img) {
        img->initialized = true;
        img->width = width;
        img->height = height;
        img->native_format = format;
        unsigned int num_pixels = img->GetWidth() * img->GetHeight();
        unsigned int num_pixels_bytes =
            num_pixels * IMAGE_FORMAT_BytesPerPixel(format);
        img->pixels[format] = new unsigned char[num_pixels_bytes];
        if (pixels) {
            memcpy(img->pixels[format], pixels, num_pixels_bytes);
        } else {
            memset(img->pixels[format], 0, num_pixels_bytes);
        }

        return img;

    } else {
        return nullptr;
    }
}

const void *Image::GetPixels(IMAGE_FORMAT format) {
    if (!initialized) {
        LoadImageData();
    }

    if (initialized) {
        if (this->pixels[format]) {
            return this->pixels[format];
        }

        auto native_pixels = this->pixels[this->native_format];
        if (native_pixels) {
            static constexpr IndexedArray<IndexedArray<ImageFormatConverter, IMAGE_NUM_FORMATS>, IMAGE_NUM_FORMATS> converters = {
                {IMAGE_FORMAT_R5G6B5, {
                    {IMAGE_FORMAT_R5G6B5,       nullptr},
                    {IMAGE_FORMAT_A1R5G5B5,     nullptr},
                    {IMAGE_FORMAT_A8R8G8B8,     nullptr},
                    {IMAGE_FORMAT_R8G8B8,       nullptr},
                    {IMAGE_FORMAT_R8G8B8A8,     nullptr},
                    {IMAGE_FORMAT_A8B8G8R8,     Image_R5G6B5_to_A8B8G8R8}
                }},

                {IMAGE_FORMAT_A1R5G5B5, {
                    {IMAGE_FORMAT_R5G6B5,       nullptr},
                    {IMAGE_FORMAT_A1R5G5B5,     nullptr},
                    {IMAGE_FORMAT_A8R8G8B8,     nullptr},
                    {IMAGE_FORMAT_R8G8B8,       nullptr},
                    {IMAGE_FORMAT_R8G8B8A8,     nullptr},
                    {IMAGE_FORMAT_A8B8G8R8,     Image_A1R5G5B5_to_A8B8G8R8}
                }},

                {IMAGE_FORMAT_A8R8G8B8, {
                    {IMAGE_FORMAT_R5G6B5,       nullptr},
                    {IMAGE_FORMAT_A1R5G5B5,     nullptr},
                    {IMAGE_FORMAT_A8R8G8B8,     nullptr},
                    {IMAGE_FORMAT_R8G8B8,       nullptr},
                    {IMAGE_FORMAT_R8G8B8A8,     nullptr},
                    {IMAGE_FORMAT_A8B8G8R8,     Image_A8R8G8B8_to_A8B8G8R8}
                }},

                {IMAGE_FORMAT_R8G8B8, {
                    {IMAGE_FORMAT_R5G6B5,       nullptr},
                    {IMAGE_FORMAT_A1R5G5B5,     nullptr},
                    {IMAGE_FORMAT_A8R8G8B8,     nullptr},
                    {IMAGE_FORMAT_R8G8B8,       nullptr},
                    {IMAGE_FORMAT_R8G8B8A8,     nullptr},
                    {IMAGE_FORMAT_A8B8G8R8,     Image_R8G8B8_to_A8B8G8R8}
                }},

                {IMAGE_FORMAT_R8G8B8A8, {
                    {IMAGE_FORMAT_R5G6B5,       nullptr},
                    {IMAGE_FORMAT_A1R5G5B5,     nullptr},
                    {IMAGE_FORMAT_A8R8G8B8,     nullptr},
                    {IMAGE_FORMAT_R8G8B8,       nullptr},
                    {IMAGE_FORMAT_R8G8B8A8,     nullptr},
                    {IMAGE_FORMAT_A8B8G8R8,     nullptr}
                }},

                {IMAGE_FORMAT_A8B8G8R8, {
                    {IMAGE_FORMAT_R5G6B5,       nullptr},
                    {IMAGE_FORMAT_A1R5G5B5,     nullptr},
                    {IMAGE_FORMAT_A8R8G8B8,     Image_A8B8G8R8_to_A8R8G8B8},
                    {IMAGE_FORMAT_R8G8B8,       nullptr},
                    {IMAGE_FORMAT_R8G8B8A8,     nullptr},
                    {IMAGE_FORMAT_A8B8G8R8,     nullptr}
                }}
            };

            ImageFormatConverter cvt = converters[this->native_format][format];
            if (cvt) {
                unsigned int num_pixels = this->GetWidth() * this->GetHeight();

                void *cvt_pixels =
                    new unsigned char[num_pixels *
                                      IMAGE_FORMAT_BytesPerPixel(format)];
                if (cvt(width * height, native_pixels, cvt_pixels)) {
                    return this->pixels[format] = cvt_pixels;
                } else {
                    delete[] cvt_pixels;
                    cvt_pixels = nullptr;
                }
            } else {
                Error("No ImageConverter defined from %s to %s",
                                toString(this->native_format).c_str(),
                                toString(format).c_str());
            }
        }
    }
    return nullptr;
}


/**
 * New function.
 *
 * @return                              Returns pointer to image R8G8B8 palette. Size 3 * 256.
 */
const void *Image::GetPalette() {
    if (!initialized) {
        LoadImageData();
    }

    if (initialized) {
        return this->palette24;
    }

    return nullptr;
}

/**
 * New function.
 *
 * @return                              Returns pointer to image pixels 8 bit palette lookup. Size 1 * width * height.
 */
const void *Image::GetPalettePixels() {
    if (!initialized) {
        LoadImageData();
    }

    if (initialized) {
        return this->palettepixels;
    }

    return nullptr;
}

std::string *Image::GetName() {
    if (!loader) __debugbreak();
    return loader->GetResourceNamePtr();
}


bool Image::Release() {
    if (loader) {
        assets->ReleaseImage(loader->GetResourceName());
        assets->ReleaseSprite(loader->GetResourceName());
    }

    if (initialized) {
        if (loader) {
            delete loader;
            loader = nullptr;
        }

        for (void *ptr : pixels)
            delete[] ptr;
        pixels.fill(nullptr);

        native_format = IMAGE_INVALID_FORMAT;
        width = 0;
        height = 0;
    }

    delete this;
    return true;
}
