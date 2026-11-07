#include "Font.h"

int Font::add(char32_t c, const GlyphMetrics &metrics, GrayscaleImageView image) {
    assert(_height > 0); // Adding to an invalid font is a coding error.
    assert(c != 0);
    assert(metrics.width == image.width());
    assert(image.height() == _height);

    int result = _indexByCharacter.value(c);
    if (result == -1) {
        result = _metrics.size();
        _indexByCharacter.insert(c, result);
        _characters.emplace_back(c);
        _metrics.emplace_back();
        _images.emplace_back();
    }

    _metrics[result] = metrics;
    _images[result] = _data.store(image.pixels()).data();
    return result;
}
