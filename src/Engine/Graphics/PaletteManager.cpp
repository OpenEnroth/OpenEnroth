#include "PaletteManager.h"

#include "Engine/Resources/ResourceManager.h"
#include "Engine/Engine.h"

#include "Library/Color/Color.h"

#include "Utility/Math/Float.h"


PaletteManager *pPaletteManager = new PaletteManager;

void PaletteManager::load(ResourceManager *resources) {
    _palettes.clear();
    _palettes.reserve(1000);

    for (int paletteId = 0; paletteId <= 999; paletteId++)
        _palettes.emplace_back(createLoadedPalette(resources->palette(paletteId)));
}

std::span<Color> PaletteManager::paletteData() {
    return {_palettes[0].colors.data(), _palettes.size() * _palettes[0].colors.size()};
}

Palette PaletteManager::createLoadedPalette(const Palette &palette) {
    float xs = engine->config->graphics.Saturation.value();
    float xv = engine->config->graphics.Lightness.value();
    if (fuzzyEquals(xs, 1.0f) && fuzzyEquals(xv, 1.0f))
        return palette;

    Palette result;
    for (size_t i = 0; i < 256; i++)
        result.colors[i] = palette.colors[i].toHsvColorf().adjusted(0, xs, xv).toColor();
    return result;
}
