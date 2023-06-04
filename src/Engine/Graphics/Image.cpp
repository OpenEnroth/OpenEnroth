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
        initialized = loader->Load(&_width, &_height, &data, &palette);
        if (initialized) {
            pixels = data;
            palette = palette;
        }
    }

    if ((_width == 0 || _height == 0) && initialized) __debugbreak();

    return initialized;
}

int GraphicsImage::width() {
    if (!initialized) {
        LoadImageData();
    }

    if (initialized) {
        if (_width == 0) __debugbreak();
        return _width;
    }

    return 0;
}

int GraphicsImage::height() {
    if (!initialized) {
        LoadImageData();
    }

    if (initialized) {
        if (_height == 0) __debugbreak();
        return _height;
    }

    return 0;
}

GraphicsImage *GraphicsImage::Create(unsigned int width, unsigned int height, const Color *pixels) {
    if (width == 0 || height == 0) __debugbreak();

    GraphicsImage *img = new GraphicsImage(false);

    img->initialized = true;
    img->_width = width;
    img->_height = height;
    unsigned int num_pixels = img->width() * img->height();
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

        _width = 0;
        _height = 0;
    }

    delete this;
    return true;
}
