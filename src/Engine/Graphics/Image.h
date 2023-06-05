#pragma once

#include <string>
#include <memory>
#include <vector>

#include "Utility/IndexedArray.h"
#include "Utility/Geometry/Size.h"
#include "Utility/Memory/Blob.h"

#include "Library/Color/Color.h"
#include "Library/Image/Image.h"
#include "Library/Image/Palette.h"

class ImageLoader;

class GraphicsImage {
 public:
    explicit GraphicsImage(bool lazy_initialization = true);
    virtual ~GraphicsImage();

    static GraphicsImage *Create(RgbaImage image);
    static GraphicsImage *Create(size_t width, size_t height);
    static GraphicsImage *Create(std::unique_ptr<ImageLoader> loader);

    size_t width();
    size_t height();
    Sizei size();

    RgbaImage &rgba();

    const Palette &palette();

    const GrayscaleImage &indexed();

    std::string *GetName();

    bool Release();

 protected:
    bool _lazyInitialization = false;
    bool _initialized = false;
    std::unique_ptr<ImageLoader> _loader;

    RgbaImage _rgbaImage;
    GrayscaleImage _indexedImage;
    Palette _palette;

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
