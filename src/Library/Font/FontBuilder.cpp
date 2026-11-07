#include "FontBuilder.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>

#include "Utility/Memory/FreeDeleter.h"

FontBuilder::FontBuilder(int height) : _height(height) {
    assert(height > 0);
}

void FontBuilder::add(char32_t c, const GlyphMetrics &metrics, GrayscaleImageView image) {
    assert(c != 0);
    assert(metrics.width == image.width());
    assert(image.height() == _height);

    GrayscaleImage copy = GrayscaleImage::copy(image.pixels().data(), image.width(), image.height());

    int index = _indexByCharacter.value(c);
    if (index != -1) {
        _metrics[index] = metrics;
        _images[index] = std::move(copy);
        return;
    }

    _indexByCharacter.insert(c, _characters.size());
    _characters.push_back(c);
    _metrics.push_back(metrics);
    _images.push_back(std::move(copy));
}

Font FontBuilder::build() && {
    std::vector<size_t> offsets;
    size_t size = 0;
    for (const GrayscaleImage &image : _images) {
        offsets.push_back(size);
        size += image.pixels().size();
    }

    std::unique_ptr<uint8_t, FreeDeleter> pixels(static_cast<uint8_t *>(std::malloc(size)));
    for (size_t i = 0; i < _images.size(); i++)
        if (!_images[i].pixels().empty())
            std::memcpy(pixels.get() + offsets[i], _images[i].pixels().data(), _images[i].pixels().size());

    return Font::fromData(_height, std::move(_characters), std::move(_metrics), std::move(offsets),
                          Blob::fromMalloc(std::move(pixels), size));
}
