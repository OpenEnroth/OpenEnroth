#include <algorithm>

#include "PaletteManager.h"
#include "Engine/Graphics/PaletteManager.h"

#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/OurMath.h"

PaletteManager *pPaletteManager = new PaletteManager;

//----- (0048A643) --------------------------------------------------------
bool HSV2RGB(float *redo, float *greeno, float *blueo, float hin, float sin, float vin) {
    // r,g,b outputs 0-1

    if (hin > 360 || sin > 1 || vin > 1) __debugbreak();

    if (sin == 0.0) {
        if (vin > 1) vin = 1;
        if (vin < 0) vin = 0;
        *blueo = vin;
        *greeno = vin;
        *redo = vin;
         return 1;
    }

    if (hin == 360.0) hin = 0.0;

    double hh = hin / 60;  // to sixth segments
    unsigned int segment = (unsigned int)hh;
    double fractionrem = hh - segment;
    double p = (1.0 - sin) * vin;
    double q = (1.0 - fractionrem * sin) * vin;
    double t = (1.0 - (1.0 - fractionrem) * sin) * vin;

    switch (segment) {
        case 0:
            *redo = vin;
            *greeno = t;
            *blueo = p;
            break;
        case 1:
            *redo = q;
            *greeno = vin;
            *blueo = p;
            break;
        case 2:
            *redo = p;
            *greeno = vin;
            *blueo = t;
            break;

        case 3:
            *redo = p;
            *greeno = q;
            *blueo = vin;
            break;
        case 4:
            *redo = t;
            *greeno = p;
            *blueo = vin;
            break;
        case 5:
        default:
            *redo = vin;
            *greeno = p;
            *blueo = q;
            break;
    }

    return 1;
}

//----- (0048A7AA) --------------------------------------------------------
void RGB2HSV(float redin, float greenin, float bluein, float *outh, float *outs, float *outv) {
    // RGB inputs 0-1
    if (redin > 1.0f || greenin > 1.0f || bluein > 1.0f) __debugbreak();

    float max = std::max({ redin, greenin, bluein });
    float min = std::min({ redin, greenin, bluein });
    float delta = max - min;

    // value
    *outv = max;

    // saturation
    if (max == 0.0) {  // r=g=b=0
        *outs = 0.0;
    } else {
        *outs = delta / max;
    }

    float outhcalc{0.0f};

    // hue
    if (max == min) {
        *outh = 0.0f;
    } else if (max == redin) {
        outhcalc = (greenin - bluein) / delta;   // yellow and mag
    } else if (max == greenin) {
        outhcalc = (bluein - redin) / delta + 2.0;    // cyan and yellow
    } else {
        outhcalc = (redin - greenin) / delta + 4.0;  // mag and cyan
    }

    *outh = outhcalc * 60.0f;  // to degree
    if (*outh < 0.0f) {
        *outh += 360.0f;
    }
}

//----- (0048A959) --------------------------------------------------------
int ReplaceHSV(unsigned int uColor, float h_replace, float s_replace, float v_replace) {
    float b = ((uColor & 0x00FF0000) >> 16) / 255.0f,
        g = ((uColor & 0x0000FF00) >> 8) / 255.0f,
        r = (uColor & 0x000000FF) / 255.0f;

    float h, s, v;
    RGB2HSV(r, g, b, &h, &s, &v);

    if (h_replace != -1.0) h = h_replace;
    if (s_replace != -1.0) s = s_replace;
    if (v_replace != -1.0) v = v_replace;
    HSV2RGB(&r, &g, &b, h, s, v);

    return (((uint)round_to_int(b * 255.0f) & 0xFF) << 16) |
        (((uint)round_to_int(g * 255.0f) & 0xFF) << 8) |
        (((uint)round_to_int(r * 255.0f) & 0xFF));
}

//----- (0048A300) --------------------------------------------------------
PaletteManager::PaletteManager() {
    Reset();
}

// new
void PaletteManager::Reset() {
    // create grayscale palette
    for (uint i = 0; i < 256; ++i) {
        pBaseColors[0][i][0] = i;
        pBaseColors[0][i][1] = i;
        pBaseColors[0][i][2] = i;
    }

    // blank paletteids
    memset(pPaletteIDs, 0, sizeof(pPaletteIDs));

    // blank sprites saved palette index
    if (pSpriteFrameTable)
        pSpriteFrameTable->ResetPaletteIndexes();

    palettestorechanged = true;
}

//----- (0048A3BC) --------------------------------------------------------
int PaletteManager::LoadPalette(unsigned int uPaletteID) {
    // Search through loaded palettes first.
    // Start at 1 as palette 0 is a grayscale palette.
    for (int i = 1; i < 50; i++)
        if (this->pPaletteIDs[i] == uPaletteID)
            return i;

    // not found in list so load
    char Source[32];
    sprintf(Source, "pal%03i", uPaletteID);

    Texture_MM7 tex;
    if (pBitmaps_LOD->LoadTextureFromLOD(&tex, Source, TEXTURE_24BIT_PALETTE) != 1)
        return 0;

    char colourstore[768]{};
    for (int index = 0; index < 768; index += 3) {
        float red = tex.pPalette24[index] / 255.0f;
        float green = tex.pPalette24[index + 1] / 255.0f;
        float blue = tex.pPalette24[index + 2] / 255.0f;

        float hue;
        float saturation;
        float value;
        RGB2HSV(red, green, blue, &hue, &saturation, &value);

        value = std::clamp(value * 1.1f, 0.0f, 1.0f);
        saturation = std::clamp(saturation * 0.64999998f, 0.0f, 1.0f);

        // covert back and store
        HSV2RGB(&red, &green, &blue, hue, saturation, value);
        colourstore[index] = static_cast<uint8_t>(red * 255.0);
        colourstore[index + 1] = static_cast<uint8_t>(green * 255.0);
        colourstore[index + 2] = static_cast<uint8_t>(blue * 255.0);
    }

    tex.Release();
    return this->MakeBasePaletteLut(uPaletteID, colourstore);
}
// 48A3BC: using guessed type char var_386[766];

//----- (0048A5A4) --------------------------------------------------------
int PaletteManager::MakeBasePaletteLut(int uPaletteID, char *entries) {
    for (int i = 0; i < 50; ++i)
        if (pPaletteIDs[i] == uPaletteID)
            return i;

    // Starting from 1 as palette at 0 is a grayscale palette
    int freeIdx = 0;
    for (int i = 1; i < 50; i++) {
        if (pPaletteIDs[i] == 0) {
            freeIdx = i;
            break;
        }
    }

    if (freeIdx == 0) {
        logger->warning("No free palette slot!");
        return 0;
    }

    memcpy(&pBaseColors[freeIdx][0][0], entries, 768);

    for (int x = 0; x < 256; x++) {
        uint8_t r = pBaseColors[freeIdx][x][0];
        uint8_t g = pBaseColors[freeIdx][x][1];
        uint8_t b = pBaseColors[freeIdx][x][2];
        p32ARGBpalette[freeIdx][x] = 255 << 24 | b << 16 | g << 8 | r;
    }

    pPaletteIDs[freeIdx] = uPaletteID;
    palettestorechanged = true;

    return freeIdx;
}


// new
int PaletteManager::GetPaletteIndex(int uPaletteID) {
    if (!uPaletteID) return 0;

    // always attempt to load a missing palette
    int ind = LoadPalette(uPaletteID);
    if (ind) return ind;

    logger->warning("Palette {} not loaded. Returning Index to greyscale!", uPaletteID);
    return 0;
}

bool PaletteManager::GetGLPaletteNeedsUpdate() {
    return palettestorechanged;
}

void PaletteManager::GLPaletteReset() {
    palettestorechanged = false;
}

size_t PaletteManager::GetGLPaletteSize() {
    return sizeof(p32ARGBpalette);
}

uint32_t *PaletteManager::GetGLPalettePtr() {
    return &p32ARGBpalette[0][0];
}

//----- (0041F50D) --------------------------------------------------------
//uint16_t *PaletteManager::Get_Dark_or_Red_LUT(int paletteIdx, int a2, char a3) {
//    int v3;  // eax@4
//
//    if (a3 & 2 || engine->config->graphics.SoftwareModeRules.Get() && engine->config->graphics.AlternativePaletteMode.Get())
//        v3 = 32 * paletteIdx + a2 + 3275;
//    else
//        v3 = 32 * paletteIdx + a2 + 75;
//    return (uint16_t *)((char *)&pPaletteManager + 512 * v3);
//}
// 4D864C: using guessed type char _4D864C_force_sw_render_rules;

