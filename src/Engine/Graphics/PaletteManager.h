#pragma once

#include <cstdint>

#include "Engine/Graphics/Sprites.h" 



/*  148 */
#pragma pack(push, 1)
struct PaletteManager {
    PaletteManager();

    void Reset();

    int LoadPalette(unsigned int uPaletteID);
    int MakeBasePaletteLut(int uPaletteID, char *entries);
    int GetPaletteIndex(int paletteID);

    bool GetGLPaletteNeedsUpdate();
    void GLPaletteReset();

    size_t GetGLPaletteSize();
    uint32_t *GetGLPalettePtr();

private:
    int pPaletteIDs[50];
    // palette / colour / rgb
    uint8_t pBaseColors[50][256][3];
    uint32_t p32ARGBpalette[50][256]{};
    bool palettestorechanged{ true };
};
#pragma pack(pop)

bool HSV2RGB(float* redo, float* greeno, float* blueo, float hin, float sin, float vin);
void RGB2HSV(float redin, float greenin, float bluein, float* outh, float* outs, float* outv);
signed int ReplaceHSV(unsigned int uColor, float a2, float gamma, float a4);
extern PaletteManager *pPaletteManager;
