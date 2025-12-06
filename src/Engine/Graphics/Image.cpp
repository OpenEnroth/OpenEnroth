#include "Engine/Graphics/Image.h"

#include <cassert>
#include <utility>
#include <memory>
#include <string>

#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/AssetsManager.h"

GraphicsImage::GraphicsImage() = default;
GraphicsImage::~GraphicsImage() = default;

GraphicsImage *GraphicsImage::Create(RgbaImage image) {
    GraphicsImage *result = new GraphicsImage();
    result->_initialized = true;
    result->_rgba = std::move(image);
    result->_renderId = render->CreateTexture(result->_rgba);
    return result;
}

GraphicsImage *GraphicsImage::Create(ssize_t width, ssize_t height) {
    assert(width > 0 && height > 0);
    return Create(RgbaImage::solid(width, height, Color()));
}

GraphicsImage *GraphicsImage::Create(Sizei size) {
    return Create(size.w, size.h);
}

GraphicsImage *GraphicsImage::Create(std::unique_ptr<ImageLoader> loader) {
    GraphicsImage *result = new GraphicsImage();
    result->_name = loader->GetResourceName();
    result->_loader = std::move(loader);
    return result;
}

ssize_t GraphicsImage::width() {
    return rgba().width();
}

ssize_t GraphicsImage::height() {
    return rgba().height();
}

Sizei GraphicsImage::size() {
    return rgba().size();
}

RgbaImage &GraphicsImage::rgba() {
    initialize();
    return _rgba;
}

const std::string &GraphicsImage::name() {
    return _name;
}

void GraphicsImage::release() {
    if (_loader) {
        if (!assets->releaseSprite(_loader->GetResourceName()))
            if (!assets->releaseImage(_loader->GetResourceName()))
                assets->releaseBitmap(_loader->GetResourceName());
    }

    releaseRenderId();

    delete this;
}

[[nodiscard]] TextureRenderId GraphicsImage::renderId() {
    if (!_renderId) {
        initialize();
        _renderId = render->CreateTexture(_rgba);
    }

    return _renderId;
}

void GraphicsImage::releaseRenderId() {
    if (!_renderId)
        return;

    render->DeleteTexture(_renderId);
    _renderId = TextureRenderId();
}

bool GraphicsImage::initialize() {
    if (_initialized)
        return true;

    assert(_loader);
    _initialized = _loader->Load(&_rgba);
    // TODO(captainurist): _initialized == false happens, investigate

    return _initialized;
}
