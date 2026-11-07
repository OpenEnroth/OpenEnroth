#include "UnicodeFont.h"

#include <cassert>
#include <utility>

#include "Utility/String/Unicode.h"

static bool hasNonZeroPixels(GrayscaleImageView img) {
    for (uint8_t p : img.pixels())
        if (p != 0)
            return true;
    return false;
}

void UnicodeFont::addChunk(const std::array<char32_t, 256> &characters, LodFont &&font) {
    assert(_fonts.size() < 256);
    assert(_fonts.empty() || _fonts[0].height() == font.height());

    for (int i = 0; i < 256; i++) {
        if (!font.supports(i))
            continue;

        // For non-space characters, check if the glyph has any non-zero pixels.
        // Some fonts have blank glyphs that should be treated as unsupported.
        if (!unicode::isSpace(characters[i]) && !hasNonZeroPixels(font.image(static_cast<char>(i))))
            continue;

        _characters.push_back(characters[i]);
        _indexByCharacter.insert(characters[i], Index(_fonts.size(), i));
    }

    _fonts.emplace_back(std::move(font));
}
