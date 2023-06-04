#include "Engine/Graphics/Image.h"

#include <algorithm>

#include "Engine/Engine.h"

#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Texture.h"

#include "Engine/Serialization/LegacyImages.h"

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
        Color *palette = nullptr;
        initialized = loader->Load(&width, &height, &data, &palette);
        if (initialized) {
            pixels = data;
            palette = palette;
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


const Color *GraphicsImage::GetPalette() {
    if (!initialized)
        LoadImageData();

    return this->palette;
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
        delete[] palette;
        delete[] palettepixels;

        width = 0;
        height = 0;
    }

    delete this;
    return true;
}
