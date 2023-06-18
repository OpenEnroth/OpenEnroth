#include "Engine/Graphics/Image.h"

#include <cassert>
#include <utility>

#include "Engine/Engine.h"

#include "Engine/Graphics/ImageLoader.h"

GraphicsImage::GraphicsImage(bool lazy_initialization): _lazyInitialization(lazy_initialization) {}

GraphicsImage::~GraphicsImage() = default;

GraphicsImage *GraphicsImage::Create(RgbaImage image) {
    GraphicsImage *img = new GraphicsImage(false);
    img->_initialized = true;
    img->_rgbaImage = std::move(image);
    return img;
}

GraphicsImage *GraphicsImage::Create(size_t width, size_t height) {
    assert(width != 0 && height != 0);
    return Create(RgbaImage::solid(width, height, Color()));
}

GraphicsImage *GraphicsImage::Create(std::unique_ptr<ImageLoader> loader) {
    GraphicsImage *img = new GraphicsImage();
    img->_loader = std::move(loader);
    return img;
}

size_t GraphicsImage::width() {
    return rgba().width();
}

size_t GraphicsImage::height() {
    return rgba().height();
}

Sizei GraphicsImage::size() {
    return rgba().size();
}

RgbaImage &GraphicsImage::rgba() {
    LoadImageData();
    return _rgbaImage;
}

const Palette &GraphicsImage::palette() {
    LoadImageData();
    return _palette;
}

const GrayscaleImage &GraphicsImage::indexed() {
    LoadImageData();
    return _indexedImage;
}

std::string *GraphicsImage::GetName() {
    assert(_loader);

    return _loader->GetResourceNamePtr();
}

bool GraphicsImage::Release() {
    if (_loader) {
        if (!assets->releaseSprite(_loader->GetResourceName()))
            if (!assets->releaseImage(_loader->GetResourceName()))
                assets->releaseBitmap(_loader->GetResourceName());
    }

    delete this;
    return true;
}

bool GraphicsImage::LoadImageData() {
    if (_initialized)
        return true;

    _initialized = _loader->Load(&_rgbaImage, &_indexedImage, &_palette);
    assert(_rgbaImage);
    return _initialized;
}
