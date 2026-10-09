#include "Engine/Graphics/Image.h"

#include <cassert>
#include <utility>
#include <memory>
#include <string>

#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Renderer/Renderer.h"

GraphicsImage::GraphicsImage() = default;

GraphicsImage::~GraphicsImage() {
    if (render) // GPU textures go away with the GL context once the renderer is shut down.
        releaseRenderId();
}

std::unique_ptr<GraphicsImage> GraphicsImage::Create(RgbaImage image) {
    std::unique_ptr<GraphicsImage> result(new GraphicsImage());
    result->_initialized = true;
    result->_rgba = std::move(image);
    result->_renderId = render->CreateTexture(result->_rgba);
    return result;
}

std::unique_ptr<GraphicsImage> GraphicsImage::Create(int width, int height) {
    assert(width > 0 && height > 0);
    return Create(RgbaImage::solid(Color(), width, height));
}

std::unique_ptr<GraphicsImage> GraphicsImage::Create(Sizei size) {
    return Create(size.w, size.h);
}

std::unique_ptr<GraphicsImage> GraphicsImage::Create(std::unique_ptr<ImageLoader> loader) {
    std::unique_ptr<GraphicsImage> result(new GraphicsImage());
    result->_name = loader->name();
    result->_loader = std::move(loader);
    return result;
}

int GraphicsImage::width() {
    return size().w;
}

int GraphicsImage::height() {
    return size().h;
}

Sizei GraphicsImage::size() {
    if (_initialized)
        return _rgba.size();

    if (!_loaderSize && _loader)
        _loaderSize = _loader->size();
    if (_loaderSize)
        return _loaderSize;

    return rgba().size();
}

RgbaImage &GraphicsImage::rgba() {
    initialize();
    return _rgba;
}

const std::string &GraphicsImage::name() {
    return _name;
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
    _rgba = _loader->load();
    _initialized = static_cast<bool>(_rgba);
    assert(!_initialized || !_loaderSize || _rgba.size() == _loaderSize);
    // TODO(captainurist): _initialized == false happens, investigate

    return _initialized;
}
