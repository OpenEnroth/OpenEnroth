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
    result->_rgba = std::move(image);
    result->_renderId = render->CreateTexture(*result->_rgba);
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
    if (_rgba)
        return _rgba->size();

    if (!_size)
        _size = _loader->loadSize();
    return *_size ? *_size : rgba().size();
}

RgbaImage &GraphicsImage::rgba() {
    if (!_rgba) {
        _rgba = _loader->loadRgba();
        assert(!*_rgba || !_size || !*_size || _rgba->size() == *_size);
    }
    return *_rgba;
}

const std::string &GraphicsImage::name() {
    return _name;
}

[[nodiscard]] TextureRenderId GraphicsImage::renderId() {
    if (!_renderId)
        _renderId = render->CreateTexture(rgba());

    return _renderId;
}

void GraphicsImage::releaseRenderId() {
    if (!_renderId)
        return;

    render->DeleteTexture(_renderId);
    _renderId = TextureRenderId();
}
