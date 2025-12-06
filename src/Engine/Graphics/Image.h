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
    static GraphicsImage *Create(RgbaImage image);
    static GraphicsImage *Create(ssize_t width, ssize_t height);
    static GraphicsImage *Create(Sizei size);
    static GraphicsImage *Create(std::unique_ptr<ImageLoader> loader);

    ssize_t width();
    ssize_t height();
    Sizei size();

    RgbaImage &rgba();

    const std::string &name();

    void release(); // TODO(captainurist): drop

    [[nodiscard]] TextureRenderId renderId();
    void releaseRenderId();

 private:
    GraphicsImage();
    ~GraphicsImage(); // Call Release() instead.

    bool initialize();

 private:
    bool _initialized = false;
    std::string _name;
    std::unique_ptr<ImageLoader> _loader;
    RgbaImage _rgba;
    TextureRenderId _renderId;
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
