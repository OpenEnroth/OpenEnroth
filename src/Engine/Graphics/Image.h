#pragma once

#include <string>
#include <memory>

#include "Engine/Graphics/Renderer/TextureRenderId.h"

#include "Library/Geometry/Size.h"
#include "Library/Image/Image.h"
#include "Library/Image/Palette.h"

#include "Utility/Types.h"

class ImageLoader;

class GraphicsImage {
 public:
    explicit GraphicsImage(bool lazy_initialization = true);

    static GraphicsImage *Create(RgbaImage image);
    static GraphicsImage *Create(ssize_t width, ssize_t height);
    static GraphicsImage *Create(Sizei size);
    static GraphicsImage *Create(std::unique_ptr<ImageLoader> loader);

    ssize_t width();
    ssize_t height();
    Sizei size();

    RgbaImage &rgba();

    const Palette &palette();

    const GrayscaleImage &indexed();

    const std::string &GetName();

    void Release();

    [[nodiscard]] TextureRenderId renderId(bool load = true);
    void releaseRenderId();

 protected:
    ~GraphicsImage(); // Call Release() instead.

 protected:
    bool _lazyInitialization = false;
    bool _initialized = false;
    std::unique_ptr<ImageLoader> _loader;
    std::string _name;

    RgbaImage _rgbaImage;
    GrayscaleImage _indexedImage;
    Palette _palette;
    TextureRenderId _renderId;

    bool LoadImageData();
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
