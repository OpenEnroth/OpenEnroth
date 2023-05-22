#pragma once

#include <span>
#include <array>
#include <vector>

#include "Engine/Graphics/Sprites.h"

class LODFile_IconsBitmaps;

struct Palette {
    std::array<uint32_t, 256> colors;
};

class PaletteManager {
 public:
    void load(LODFile_IconsBitmaps *lod);

    /**
     * @param paletteId                 Palette identifier, a number in [0, 999].
     * @return                          Index for the provided palette identifier. Returned index can then be used
     *                                  for getting palette data from the return value of `paletteData` function.
     */
    int paletteIndex(int paletteId);

    /**
     * @return                          Span containing contiguous data for all loaded palettes.
     */
    std::span<uint32_t> paletteData();

 private:
    static Palette createGrayscalePalette();
    static Palette createLoadedPalette(uint8_t *data);

 private:
    std::vector<int> _paletteIds;
    std::vector<Palette> _palettes;
};

bool HSV2RGB(float *redo, float *greeno, float *blueo, float hin, float sin, float vin);
void RGB2HSV(float redin, float greenin, float bluein, float *outh, float *outs, float *outv);
signed int ReplaceHSV(unsigned int uColor, float a2, float gamma, float a4);
extern PaletteManager *pPaletteManager;
