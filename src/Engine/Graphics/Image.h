#pragma once

#include <string>
#include <vector>

#include "Utility/IndexedArray.h"
#include "Utility/Geometry/Size.h"
#include "Utility/Memory/Blob.h"

#include "Library/Color/Color.h"

class ImageLoader;
class GraphicsImage {
 public:
    explicit GraphicsImage(bool lazy_initialization = true): lazy_initialization(lazy_initialization) {}
    virtual ~GraphicsImage() {}

    static GraphicsImage *Create(unsigned int width, unsigned int height, const Color *pixels = nullptr);
    static GraphicsImage *Create(ImageLoader *loader);

    int width();
    int height();
    Sizei size() { return {width(), height()}; }

    const Color *GetPixels();

    /**
     * @return                              Returns pointer to image R8G8B8 palette. Size 3 * 256.
     */
    const Color *GetPalette();

    /**
     * @return                              Returns pointer to image pixels 8 bit palette lookup. Size 1 * width * height.
     */
    const uint8_t *GetPalettePixels();

    std::string *GetName();

    bool Release();

 protected:
    bool lazy_initialization = false;
    bool initialized = false;
    ImageLoader *loader = nullptr;

    size_t _width = 0;
    size_t _height = 0;
    Color *pixels = nullptr;
    Color *palette = nullptr;
    uint8_t *palettepixels = nullptr;

    virtual bool LoadImageData();
};

class ImageHelper {
 public:
    static int GetWidthLn2(GraphicsImage *img) {
        return ImageHelper::GetPowerOf2(img->width());
    }

    static int GetHeightLn2(GraphicsImage *img) {
        return ImageHelper::GetPowerOf2(img->height());
    }

    static int GetPowerOf2(int value) {
        int power = 1;
        while (1 << power != value) {
            ++power;
        }

        return power;
    }
};
