#include "PaletteManager.h"

#include <algorithm>
#include <string>

#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/OurMath.h"

#include "Library/Color/Color.h"
#include "Utility/Format.h"

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

void PaletteManager::load(LODFile_IconsBitmaps *lod) {
    // Palette #0 is grayscale.
    _paletteIds.push_back(0);
    _palettes.push_back(createGrayscalePalette());

    // Load all other palettes.
    for (int paletteId = 1; paletteId <= 999; paletteId++) {
        std::string paletteName = fmt::format("pal{:03}", paletteId);

        Texture_MM7 texture;
        if (lod->LoadTextureFromLOD(&texture, paletteName, TEXTURE_24BIT_PALETTE) != 1)
            continue;

        _paletteIds.push_back(paletteId);
        _palettes.push_back(createLoadedPalette(texture.pPalette24));

        texture.Release();
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

Palette PaletteManager::createLoadedPalette(uint8_t *data) {
    Palette result;

    for (int index = 0; index < 768; index += 3) {
        float red = data[index] / 255.0f;
        float green = data[index + 1] / 255.0f;
        float blue = data[index + 2] / 255.0f;

        float hue;
        float saturation;
        float value;
        RGB2HSV(red, green, blue, &hue, &saturation, &value);

        value = std::clamp(value * 1.1f, 0.0f, 1.0f);
        saturation = std::clamp(saturation * 0.64999998f, 0.0f, 1.0f);

        HSV2RGB(&red, &green, &blue, hue, saturation, value);

        result.colors[index / 3] = Color(red * 255.0, green * 255.0, blue * 255.0);
    }

    return result;
}
