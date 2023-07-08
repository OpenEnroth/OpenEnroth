#include "Engine/Graphics/Image.h"

#include <cassert>
#include <utility>

#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"

#include "Engine/Graphics/ImageLoader.h"

#include "IRender.h"

GraphicsImage::GraphicsImage(bool lazy_initialization): _lazyInitialization(lazy_initialization) {}

GraphicsImage::~GraphicsImage() = default;

GraphicsImage *GraphicsImage::Create(RgbaImage image) {
    GraphicsImage *result = new GraphicsImage(false);
    result->_initialized = true;
    result->_rgbaImage = std::move(image);
    result->_renderId = render->CreateTexture(result->_rgbaImage);
    return result;
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

void GraphicsImage::Release() {
    if (_loader) {
        if (!assets->releaseSprite(_loader->GetResourceName()))
            if (!assets->releaseImage(_loader->GetResourceName()))
                assets->releaseBitmap(_loader->GetResourceName());
    }

    releaseRenderId();

    delete this;
}

[[nodiscard]] TextureRenderId GraphicsImage::renderId(bool load) {
    if (load) {
        LoadImageData();
        if (!_renderId)
            _renderId = render->CreateTexture(_rgbaImage);
    }

    return _renderId;
}

void GraphicsImage::releaseRenderId() {
    if (!_renderId)
        return;

    render->DeleteTexture(_renderId);
    _renderId = TextureRenderId();
}

bool GraphicsImage::LoadImageData() {
    if (_initialized)
        return true;

    _initialized = _loader->Load(&_rgbaImage, &_indexedImage, &_palette);
    // TODO(captainurist): _initialized == false happens, investigate

    if (_initialized)
        _renderId = render->CreateTexture(_rgbaImage);

    return _initialized;
}
