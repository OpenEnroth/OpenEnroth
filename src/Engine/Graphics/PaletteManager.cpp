#include "PaletteManager.h"

#include <algorithm>
#include <string>

#include "Engine/Graphics/Texture_MM7.h"
#include "Engine/LodTextureCache.h"

#include "Library/Color/Color.h"
#include "Library/Logger/Logger.h"

#include "Utility/String/Format.h"

PaletteManager *pPaletteManager = new PaletteManager;

void PaletteManager::load(LodTextureCache *lod) {
    // Palette #0 is grayscale.
    _paletteIds.push_back(0);
    _palettes.push_back(createGrayscalePalette());

    // Load all other palettes.
    for (int paletteId = 1; paletteId <= 999; paletteId++) {
        std::string paletteName = fmt::format("pal{:03}", paletteId);

        Texture_MM7 *texture = lod->loadTexture(paletteName, false);
        if (!texture)
            continue;

        _paletteIds.push_back(paletteId);
        _palettes.push_back(createLoadedPalette(texture->palette));

        texture->Release();
    }
}

int PaletteManager::paletteIndex(int paletteId) {
    auto pos = std::lower_bound(_paletteIds.begin(), _paletteIds.end(), paletteId);

    if (pos == _paletteIds.end() || *pos != paletteId) {
        logger->warning("Palette {} doesn't exist. Returning index to grayscale!", paletteId);
        return 0;
    }

    return pos - _paletteIds.begin();
}

std::span<Color> PaletteManager::paletteData() {
    return {_palettes[0].colors.data(), _palettes.size() * _palettes[0].colors.size()};
}

Palette PaletteManager::createGrayscalePalette() {
    Palette result;
    for (int i = 0; i < 256; i++)
        result.colors[i] = Color(i, i, i, 255);
    return result;
}

Palette PaletteManager::createLoadedPalette(const Palette &palette) {
    Palette result;

    for (size_t i = 0; i < 256; i++) {
        HsvColorf hsv = palette.colors[i].toColorf().toHsv();

        hsv.v = std::clamp(hsv.v * 1.1f, 0.0f, 1.0f);
        hsv.s = std::clamp(hsv.s * 0.64999998f, 0.0f, 1.0f);

        result.colors[i] = hsv.toRgb().toColor();
    }

    return result;
}
