#include "Font.h"

#include <utility>
#include <vector>

Font Font::fromData(int height, std::vector<char32_t> characters, std::vector<GlyphMetrics> metrics,
                    std::vector<size_t> offsets, Blob pixels) {
    assert(height > 0);
    assert(characters.size() == metrics.size() && characters.size() == offsets.size());

    Font result;
    result._height = height;
    for (size_t i = 0; i < characters.size(); i++) {
        assert(characters[i] != 0 && result._indexByCharacter.value(characters[i]) == -1);
        assert(metrics[i].width >= 0 && offsets[i] + static_cast<size_t>(metrics[i].width) * height <= pixels.size());
        result._indexByCharacter.insert(characters[i], static_cast<int>(i));
    }
    result._characters = std::move(characters);
    result._metrics = std::move(metrics);
    result._offsets = std::move(offsets);
    result._pixels = std::move(pixels);
    return result;
}
