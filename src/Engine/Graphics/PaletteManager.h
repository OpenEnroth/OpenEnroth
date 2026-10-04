#pragma once

#include <span>
#include <vector>

#include "Library/Image/Palette.h"

class ResourceManager;

class PaletteManager {
 public:
    void load(ResourceManager *resources);

    /**
     * @return                          Span containing contiguous data for all 1000 loaded palettes.
     */
    std::span<Color> paletteData();

    static Palette createLoadedPalette(const Palette &palette);

 private:
    std::vector<Palette> _palettes;
};

extern PaletteManager *pPaletteManager;
