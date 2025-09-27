#include "PaletteManager.h"

#include <algorithm>
#include <string>

#include "Engine/LodTextureCache.h"
#include "Engine/Engine.h"

#include "Library/Color/Color.h"
#include "Library/LodFormats/LodImage.h"
#include "Library/Logger/Logger.h"

#include "Utility/String/Format.h"


PaletteManager *pPaletteManager = new PaletteManager;

void PaletteManager::load(LodTextureCache *lod) {
    _palettes.clear();
    _palettes.reserve(1000);

    // Palette #0 is grayscale.
    _palettes.emplace_back(createGrayscalePalette());

    // Load all other palettes.
    for (int paletteId = 1; paletteId <= 999; paletteId++) {
        std::string paletteName = fmt::format("pal{:03}", paletteId);

        LodImage *texture = lod->loadTexture(paletteName, false);
        if (texture) {
            _palettes.emplace_back(createLoadedPalette(texture->palette));
        } else {
            _palettes.emplace_back(createGrayscalePalette());
        }
    }
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
    float xs = engine->config->graphics.Saturation.value();
    float xv = engine->config->graphics.Lightness.value();

    Palette result;
    for (size_t i = 0; i < 256; i++)
        result.colors[i] = palette.colors[i].toHsvColorf().adjusted(0, xs, xv).toColor();
    return result;
}
