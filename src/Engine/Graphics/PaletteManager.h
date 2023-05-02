#pragma once

#include <cstdint>

#include "Engine/Graphics/Sprites.h"

struct PaletteManager {
    PaletteManager();

    /**
     * Resets the currently loaded Palettes and clears sprite indexes.
     */
    void Reset();

    int LoadPalette(unsigned int uPaletteID);
    int MakeBasePaletteLut(int uPaletteID, char *entries);
    int GetPaletteIndex(int paletteID);

    /**
     * @return                              Returns true if palette needs reloading to shader.
     */
    bool GetGLPaletteNeedsUpdate();
    /**
     * Resets the shader palette reload needed flag.
     */
    void GLPaletteReset();

    /**
     * @return                              Returns size of the shader palette store.
     */
    size_t GetGLPaletteSize();
    /**
     * @return                              Returns pointer to the palette data store for uploading to shader.
     */
    uint32_t *GetGLPalettePtr();

 private:
    int pPaletteIDs[50];
    // palette / colour / rgb
    uint8_t pBaseColors[50][256][3];
    uint32_t p32ARGBpalette[50][256]{};
    bool palettestorechanged{ true };
};

bool HSV2RGB(float *redo, float *greeno, float *blueo, float hin, float sin, float vin);
void RGB2HSV(float redin, float greenin, float bluein, float *outh, float *outs, float *outv);
signed int ReplaceHSV(unsigned int uColor, float a2, float gamma, float a4);
extern PaletteManager *pPaletteManager;
