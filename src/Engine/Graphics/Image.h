#pragma once

#include <string>
#include <memory>

#include "Engine/Graphics/Renderer/TextureRenderId.h"

#include "Library/Geometry/Size.h"
#include "Library/Image/Image.h"
#include "Library/Image/Palette.h"

class ImageLoader;

class GraphicsImage {
 public:
    static GraphicsImage *Create(RgbaImage image);
    static GraphicsImage *Create(int width, int height);
    static GraphicsImage *Create(Sizei size);
    static GraphicsImage *Create(std::unique_ptr<ImageLoader> loader);

    int width();
    int height();
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
