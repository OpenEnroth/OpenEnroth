#pragma once

#include <span>
#include <vector>

#include "Library/Image/Palette.h"

class LodTextureCache;

class PaletteManager {
 public:
    void load(LodTextureCache *lod);

    /**
     * @return                          Span containing contiguous data for all 1000 loaded palettes.
     */
    std::span<Color> paletteData();

    static Palette createGrayscalePalette();
    static Palette createLoadedPalette(const Palette &palette);

 private:
    std::vector<Palette> _palettes;
    std::vector<int> _rawPaletteIds; // Palettes from the LOD that paletteData() hasn't desaturated yet.
};

extern PaletteManager *pPaletteManager;
