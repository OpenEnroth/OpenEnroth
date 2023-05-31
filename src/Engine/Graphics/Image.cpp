#include "Engine/Graphics/Image.h"

#include <algorithm>

#include "Engine/Engine.h"

#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Texture.h"

#include "Engine/Serialization/LegacyImages.h"

#include "Library/Serialization/EnumSerialization.h"


struct TextureFrameTable *pTextureFrameTable;

Texture *TextureFrame::GetTexture() {
    if (!this->tex) {
        this->tex = assets->getBitmap(this->name);
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

int TextureFrameTable::textureFrameAnimLength(int frameID) {
    int animLength = textures[frameID].uAnimLength;
    if (textures[frameID].uFlags & 1 && animLength != 0) {
        return animLength;
    }
    return 1;
}

int TextureFrameTable::textureFrameAnimTime(int frameID) {
    return textures[frameID].uAnimTime;
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

GraphicsImage *GraphicsImage::Create(ImageLoader *loader) {
    GraphicsImage *img = new GraphicsImage();
    if (img) {
        img->loader = loader;
    }

    return img;
}

bool GraphicsImage::LoadImageData() {
    if (!initialized) {
        Color *data = nullptr;
        uint8_t *palette = nullptr;
        initialized = loader->Load(&width, &height, &data, &palette);
        if (initialized) {
            pixels = data;
            palette24 = palette;
        }
    }

    if ((width == 0 || height == 0) && initialized) __debugbreak();

    return initialized;
}

int GraphicsImage::GetWidth() {
    if (!initialized) {
        LoadImageData();
    }

    if (initialized) {
        if (width == 0) __debugbreak();
        return width;
    }

    return 0;
}

int GraphicsImage::GetHeight() {
    if (!initialized) {
        LoadImageData();
    }

    if (initialized) {
        if (height == 0) __debugbreak();
        return height;
    }

    return 0;
}

GraphicsImage *GraphicsImage::Create(unsigned int width, unsigned int height, const Color *pixels) {
    if (width == 0 || height == 0) __debugbreak();

    GraphicsImage *img = new GraphicsImage(false);

    img->initialized = true;
    img->width = width;
    img->height = height;
    unsigned int num_pixels = img->GetWidth() * img->GetHeight();
    unsigned int num_pixels_bytes = num_pixels * sizeof(Color);
    img->pixels = new Color[num_pixels];
    if (pixels) {
        memcpy(img->pixels, pixels, num_pixels_bytes);
    } else {
        memset(img->pixels, 0, num_pixels_bytes);
    }

    return img;
}

const Color *GraphicsImage::GetPixels() {
    if (!initialized)
        LoadImageData();
    return pixels;
}


const uint8_t *GraphicsImage::GetPalette() {
    if (!initialized)
        LoadImageData();

    return this->palette24;
}

const uint8_t *GraphicsImage::GetPalettePixels() {
    if (!initialized)
        LoadImageData();
    return this->palettepixels;
}

std::string *GraphicsImage::GetName() {
    if (!loader) __debugbreak();
    return loader->GetResourceNamePtr();
}

bool GraphicsImage::Release() {
    if (loader) {
        if (!assets->releaseSprite(loader->GetResourceName()))
            if (!assets->releaseImage(loader->GetResourceName()))
                assets->releaseBitmap(loader->GetResourceName());
    }

    if (initialized) {
        if (loader) {
            delete loader;
            loader = nullptr;
        }

        delete[] pixels;

        width = 0;
        height = 0;
    }

    delete this;
    return true;
}
