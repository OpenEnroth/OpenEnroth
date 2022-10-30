#include <algorithm>

#include "PaletteManager.h"
#include "Engine/Graphics/PaletteManager.h"

#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/OurMath.h"

PaletteManager *pPaletteManager = new PaletteManager;

//----- (00452AE2) --------------------------------------------------------
int MakeColorMaskFromBitDepth(const int num_bits) {
    // 2^n - 1
    return (1 << num_bits) - 1;
}

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
void RGB2HSV(float redin, float greenin, float bluein, float* outh, float* outs, float *outv) {
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

//----- (0048A8A3) --------------------------------------------------------
int PaletteManager::LockAll() {
    int *v1;        // edx@1
    signed int v2;  // eax@1

    v1 = &this->pPaletteIDs[1];
    v2 = 1;
    do {
        if (*v1) this->_num_locked = v2 + 1;
        ++v2;
        ++v1;
    } while (v2 < 50);
    return this->_num_locked;
}

//----- (0048A8CC) --------------------------------------------------------
int PaletteManager::LockTestAll() {
    char *v1;       // edx@1
    signed int v2;  // eax@1

    v1 = (char *)&this->pPaletteIDs[1];
    v2 = 1;
    do {
        if (*(int *)v1) this->_pal_lock_test = v2 + 1;
        ++v2;
        v1 += 4;
    } while (v2 < 50);
    return this->_pal_lock_test;
}

//----- (0048A8F5) --------------------------------------------------------
void PaletteManager::SetColorChannelInfo(int uNumRBits, int uNumGBits, int uNumBBits) {
    this->uNumTargetRBits = uNumRBits;
    this->uNumTargetGBits = uNumGBits;
    this->uNumTargetBBits = uNumBBits;

    this->uTargetRMask = MakeColorMaskFromBitDepth(uNumRBits) << (uNumGBits + uNumBBits);
    this->uTargetGMask = MakeColorMaskFromBitDepth(uNumGBits) << uNumBBits;
    this->uTargetBMask = MakeColorMaskFromBitDepth(uNumBBits);
}

//----- (00489BE0) --------------------------------------------------------
void PaletteManager::CalcPalettes_LUT(int paletteIdx) {
    double v26;  // st7@20
    double v32;  // st5@28
    int64_t v36;  // qax@33
    signed int v37;      // edx@33
    char v38;            // cl@33
    unsigned int v39;    // ebx@33
    signed int v40;      // edi@33
    unsigned int v41;    // ecx@33
    unsigned int v42;    // ecx@35
    // int v43; // eax@39
    signed int v44;  // edx@39
    // uint8_t v45; // al@40
    // double v46; // st6@43
    // signed int v47; // eax@43
    // int v48; // eax@45
    double v49;  // st6@47
    // float v50; // ST08_4@49
    // unsigned int v51; // ebx@49
    int outred;             // edi@55
    int outgreen;             // ebx@55
    //int64_t v54;  // qax@55
    double v55;          // st7@56
    unsigned int v56;    // ecx@57
    unsigned int v57;    // ecx@59
    // int v58; // edx@61
    unsigned int v59;  // ecx@61
    unsigned int v60;  // eax@63
    char v61;          // cl@63
    // int result; // eax@63
    float local_s[256];  // [sp+1Ch] [bp-C38h]@5
    float local_h[256];  // [sp+41Ch] [bp-838h]@5
    float local_v[256];   // [sp+81Ch] [bp-438h]@5
                     //  int v66; // [sp+C1Ch] [bp-38h]@43
    float v67;       // [sp+C20h] [bp-34h]@43
    float v68;       // [sp+C24h] [bp-30h]@43
    // PaletteManager *v69; // [sp+C28h] [bp-2Ch]@9
    // float v70; // [sp+C2Ch] [bp-28h]@43
    // double v71; // [sp+C30h] [bp-24h]@10
    // int v72; // [sp+C38h] [bp-1Ch]@9
    // int v73; // [sp+C3Ch] [bp-18h]@9
    // int i; // [sp+C40h] [bp-14h]@7
    // float v75; // [sp+C44h] [bp-10h]@5
    float fOutgreen;       // [sp+C48h] [bp-Ch]@13
    float fOutred;        // [sp+C4Ch] [bp-8h]@13
    float fOutblue;        // [sp+C50h] [bp-4h]@13
    signed int v79;  // [sp+C5Ch] [bp+8h]@33
    // signed int v80; // [sp+C5Ch] [bp+8h]@43
    int outblue;  // [sp+C5Ch] [bp+8h]@57

    // convert palette to HSV color
    // if any of these are set we are adding a tint to the palette
    if (pPalette_tintColor[0] || pPalette_tintColor[1] || pPalette_tintColor[2]) {
        for (uint i = 0; i < 256; ++i)
            RGB2HSV((pBaseColors[paletteIdx][i][0] + pPalette_tintColor[0]) / (255.0f + 255.0f),
                    (pBaseColors[paletteIdx][i][1] + pPalette_tintColor[1]) / (255.0f + 255.0f),
                    (pBaseColors[paletteIdx][i][2] + pPalette_tintColor[2]) / (255.0f + 255.0f),
                    &local_h[i], &local_s[i], &local_v[i]);
    } else {
        for (uint i = 0; i < 256; ++i)
            RGB2HSV(pBaseColors[paletteIdx][i][0] / 255.0f,
                    pBaseColors[paletteIdx][i][1] / 255.0f,
                    pBaseColors[paletteIdx][i][2] / 255.0f,
                    &local_h[i], &local_s[i], &local_v[i]);
    }

    for (uint i = 0; i < 32; ++i) {
        for (uint j = 0; j < 256; ++j) {
            float fModv = local_v[j] * (1.0f - i / 32.0f);
            if (fModv < 0.0f) fModv = 0.0f;

            HSV2RGB(&fOutred, &fOutgreen, &fOutblue, local_h[j], local_s[j], fModv);
            if (this->uNumTargetGBits == 6) {  // r5g6b5
                fOutred = fOutred * 31.0;
                fOutgreen = fOutgreen * 62.0;
                fOutblue = fOutblue * 31.0;
            } else if (this->uNumTargetGBits == 5) {  // r5g5b5
                fOutred = fOutred * 31.0;
                fOutgreen = fOutgreen * 31.0;
                fOutblue = fOutblue * 31.0;
            } else if (this->uNumTargetGBits == 0) {
                // logger->Warning("Calling palette manager with
                // num_target_bits == 0");
            } else {
                Error("(%u)", this->uNumTargetGBits);
            }

            pPalette1[paletteIdx][i][j] =
                (uint32_t)fOutblue |
                ((uint32_t)fOutgreen << this->uNumTargetBBits) |
                ((uint32_t)fOutred << (this->uNumTargetBBits + this->uNumTargetGBits));
        }
    }

    // v72 = 0;
    // v73 = (int)v69->field_199600_palettes;
    // do
    for (uint i = 0; i < 32; ++i) {
        // i = 0;
        // v71 = 1.0 - (double)v72 / 31.0f;
        // do
        for (uint j = 0; j < 256; ++j) {
            v26 = local_v[j] * (1.0 - i / 31.0f);
            if (v26 < 0.0) v26 = 0.0;

            // v27 = v26;
            HSV2RGB(&fOutred, &fOutgreen, &fOutblue, 1.0, 1.0, v26);
            if (this->uNumTargetGBits == 6) {
                fOutred = fOutred * 31.0;
                fOutgreen = fOutgreen * 62.0;
                fOutblue = fOutblue * 31.0;
            } else if (this->uNumTargetGBits == 5) {
                fOutred = fOutred * 31.0;
                fOutgreen = fOutgreen * 31.0;
                fOutblue = fOutblue * 31.0;
            } else if (this->uNumTargetGBits == 0) {
                // logger->Warning("Calling palette manager with
                // num_target_bits == 0");
            } else {
                Error("(%u)", this->uNumTargetGBits);
            }

            field_199600_palettes[paletteIdx][i][j] =
                (uint32_t)fOutblue |
                ((uint16_t)(int64_t)fOutgreen << this->uNumTargetBBits) |
                (uint16_t)((uint16_t)(int64_t)fOutred
                                   << (this->uNumTargetBBits + this->uNumTargetGBits));
        }
        // while ( v18 ^ v19 );
        // ++v72;
    }
    // while ( v72 < 32 );

    // v73 = (int)((char *)v2 + 512 * (paletteIdx + 4875));   // field_261600[paletteIdx]
    // v31 = 0;
    // i = 0;
    for (uint i = 0; i < 256; ++i) {
        // v32 = (*(float *)((char *)local_v + v31) - 0.8) * 0.8387096774193549 +
        // 0.8;
        v32 = (local_v[i] - 0.8f) * 0.8387096774193549 + 0.8;
        if (v32 < 0.0) v32 = 0.0;

        // v33 = v32;
        // v34 = local_s[i] * 0.7034339229968783;
        HSV2RGB(&fOutred, &fOutgreen, &fOutblue, local_h[i], local_s[i] * 0.7034339229968783, v32);
        if (this->uNumTargetGBits == 6) {
            fOutred = fOutred * 31.0;
            fOutgreen = fOutgreen * 62.0;
            fOutblue = fOutblue * 31.0;
        } else if (this->uNumTargetGBits == 5) {
            fOutred = fOutred * 31.0;
            fOutgreen = fOutgreen * 31.0;
            fOutblue = fOutblue * 31.0;
        } else if (this->uNumTargetGBits == 0) {
            // logger->Warning("Calling palette manager with num_target_bits
            // == 0");
        } else {
            Error("(%u)", this->uNumTargetGBits);
        }

        v36 = (int64_t)((fOutred + fOutgreen + fOutblue) * 0.3333333333333333 * 8.0);
        v37 = (signed int)v36 >> (8 - this->uNumTargetRBits);
        v38 = 8 - this->uNumTargetGBits;
        v39 = this->uNumTargetBBits;
        v40 = (signed int)v36 >> v38;
        v41 = this->uNumTargetRBits;
        v79 = (signed int)v36 >> (8 - v39);
        if (v37 > (1 << v41) - 1) v37 = (1 << v41) - 1;
        v42 = this->uNumTargetGBits;
        if (v40 > (1 << v42) - 1) v40 = (1 << v42) - 1;
        if (v79 > (1 << v39) - 1) v79 = (1 << v39) - 1;
        // v43 = v73;
        v44 = v37 << (v39 + this->uNumTargetGBits);
        // v73 += 2;
        // *(short *)v43 = v79 | ((short)v40 << v39) | (uint16_t)v44;
        field_261600[paletteIdx][i] =
            v79 | ((unsigned short)v40 << v39) | (uint16_t)v44;
        // v31 = i + 4;
        // v19 = __OFSUB__(i + 4, 1024);
        // v18 = i - 1020 < 0;
        // i += 4;
    }
    // while ( v18 ^ v19 );

    // v45 = pPalette_mistColor[0];
    float mist_a, mist_b, mist_c;
    if (pPalette_mistColor[0] || pPalette_mistColor[1] ||
        pPalette_mistColor[2]) {
        // v46 = (double)v45;
        // v80 = pPalette_mistColor[1];
        // v47 = pPalette_mistColor[2];
        // v70 = v46 * 0.00392156862745098;
        // *((float *)&v71 + 1) = (double)v80 * 0.00392156862745098;
        // v75 = (double)v47 * 0.00392156862745098;
        mist_a = pPalette_mistColor[0] / 255.0f;
        mist_b = pPalette_mistColor[1] / 255.0f;
        mist_c = pPalette_mistColor[2] / 255.0f;

        float unused;
        RGB2HSV(mist_a, mist_b, mist_c, &v68, &v67, &unused);
    }

    // v72 = 0;
    // v73 = (int)v69->field_D1600;
    // do
    for (uint i = 0; i < 32; ++i) {
        // v48 = 0;
        // for ( i = 0; ; v48 = i )
        for (uint j = 0; j < 256; ++j) {
            v49 = local_s[j];
            if (v49 < 0.0) v49 = 0.0;

            // v50 = v49;
            HSV2RGB(&fOutred, &fOutgreen, &fOutblue, local_h[j], v49, local_v[j]);
            if (this->uNumTargetGBits == 6) {
                fOutred = fOutred * 31.0;
                fOutgreen = fOutgreen * 62.0;
                fOutblue = fOutblue * 31.0;
            } else if (this->uNumTargetGBits == 5) {
                fOutred = fOutred * 31.0;
                fOutgreen = fOutgreen * 31.0;
                fOutblue = fOutblue * 31.0;
            } else if (this->uNumTargetGBits == 0) {
                // logger->Warning("Calling palette manager with
                // num_target_bits == 0");
            } else {
                Error("(%u)", this->uNumTargetGBits);
            }

            if (pPalette_mistColor[0] || pPalette_mistColor[1] || pPalette_mistColor[2]) {
                v55 = (double)i / 31.0f;
                outred = (int64_t)((double)(1 << this->uNumTargetRBits) *
                                           mist_a * v55 +
                                       fOutred * (1.0 - v55));
                outgreen = (int64_t)((double)(1 << this->uNumTargetGBits) *
                                           mist_b * v55 +
                                       fOutgreen * (1.0 - v55));
                outblue = (int64_t)((double)(1 << this->uNumTargetBBits) *
                                           mist_c * v55 +
                                       fOutblue * (1.0 - v55));
            } else {
                outred = (int64_t)fOutred;
                outgreen = (int64_t)fOutgreen;
                outblue = (int64_t)fOutblue;
            }

            if (outred > MakeColorMaskFromBitDepth(this->uNumTargetRBits))
                outred = MakeColorMaskFromBitDepth(this->uNumTargetRBits);
            if (outgreen > MakeColorMaskFromBitDepth(this->uNumTargetGBits))
                outgreen = MakeColorMaskFromBitDepth(this->uNumTargetGBits);
            if (outblue > MakeColorMaskFromBitDepth(this->uNumTargetBBits))
                outblue = MakeColorMaskFromBitDepth(this->uNumTargetBBits);

            field_D1600[paletteIdx][i][j] =
                outblue | (outgreen << this->uNumTargetBBits) | (outred << (this->uNumTargetBBits + this->uNumTargetGBits));
        }
    }
}

//----- (0048A300) --------------------------------------------------------
PaletteManager::PaletteManager()
    : uNumTargetRBits(0),
      uNumTargetGBits(0),
      uNumTargetBBits(0),
      _num_locked(0) {
    for (uint i = 0; i < 256; ++i) {
        pBaseColors[0][i][0] = i;
        pBaseColors[0][i][1] = i;
        pBaseColors[0][i][2] = i;
    }

    memset(pPaletteIDs, 0, sizeof(pPaletteIDs));
    memset(pPalette_tintColor, 0, sizeof(pPalette_tintColor));
    memset(pPalette_mistColor, 0, sizeof(pPalette_mistColor));
    CalcPalettes_LUT(0);
}

//----- (0048A336) --------------------------------------------------------
// make grayscale palette at 0, clear all palettes ids that aren't locked
int PaletteManager::ResetNonLocked() {
    PaletteManager *v1;  // esi@1
    signed int v2;       // ecx@1
    // int v3;              // eax@1
    signed int result;   // eax@3
    void *v5;            // edi@4
    int v6;              // ecx@4

    v1 = this;
    v2 = 0;

//    v3 = (int)&v1->pBaseColors[0][0][1];
//    do {
//        *(char *)(v3 + 1) = v2;
//        *(char *)v3 = v2;
//        *(char *)(v3 - 1) = v2++;
//        v3 += 3;
//    } while (v2 < 256);

    for (uint i = 0; i < 256; ++i) {
        v1->pBaseColors[0][i][0] = i;
        v1->pBaseColors[0][i][1] = i;
        v1->pBaseColors[0][i][2] = i;
    }

    CalcPalettes_LUT(0);
    result = v1->_num_locked;
    if (result < 50) {
        v5 = &v1->pPaletteIDs[result];
        v6 = 50 - result;
        result = 0;
        memset(v5, 0, sizeof(int) * v6);
    }
    return result;
}

//----- (0048A379) --------------------------------------------------------
// make grayscale palette at 0, clear all palettes ids that aren't in
// "lock_test"
int PaletteManager::ResetNonTestLocked() {
    PaletteManager *v1;  // esi@1
    signed int v2;       // ecx@1
    // int v3;              // eax@1
    signed int result;   // eax@3
    void *v5;            // edi@4
    int v6;              // ecx@4

    v1 = this;
    v2 = 0;

//    v3 = (int)&v1->pBaseColors[0][0][1];
//    do {
//        *(char *)(v3 + 1) = v2;
//        *(char *)v3 = v2;
//        *(char *)(v3 - 1) = v2++;
//        v3 += 3;
//    } while (v2 < 256);

    for (uint i = 0; i < 256; ++i) {
        v1->pBaseColors[0][i][0] = i;
        v1->pBaseColors[0][i][1] = i;
        v1->pBaseColors[0][i][2] = i;
    }

    CalcPalettes_LUT(0);
    result = v1->_pal_lock_test;
    if (result < 50) {
        v5 = &v1->pPaletteIDs[result];
        v6 = 50 - result;
        result = 0;
        memset(v5, 0, sizeof(int) * v6);
    }
    return result;
}

//----- (0048A3BC) --------------------------------------------------------
int PaletteManager::LoadPalette(unsigned int uPaletteID) {
    // Search through loaded palettes first.
    // Start at 1 as palette 0 is a grayscale palette.
    for (int i = 1; i < 50; i++)
        if (this->pPaletteIDs[i] == uPaletteID)
            return i;

    // not found in list so load
    if (engine->config->debug.VerboseLogging.Get())
        logger->Info("Loading palette %u", uPaletteID);

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
    // PaletteManager *v3; // edi@1
    // signed int result; // eax@1
    // int *v5; // ecx@1
    // int v8; // eax@9
    // signed int v9; // ecx@9
    // int v10; // edx@9

    // v3 = this;
    // result = 0;
    // v5 = this->pPaletteIDs;

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
    if (freeIdx == 0)
        return 0;

    /*v8 = (int)pBaseColors[v7];//(int)((char *)v3 + 768 * v7);
    v9 = 768;
    v10 = (int)(entries - v8);
    do
    {
      *(char *)v8 = *(char *)(v10 + v8);
      ++v8;
      --v9;
    }
    while ( v9 );*/
    memcpy(&pBaseColors[freeIdx][0][0], entries, 768);

    for (int x = 0; x < 256; x++) {
        uint8_t r = pBaseColors[freeIdx][x][0];
        uint8_t g = pBaseColors[freeIdx][x][1];
        uint8_t b = pBaseColors[freeIdx][x][2];
        p32ARGBpalette[freeIdx][x] = 255 << 24 | b << 16 | g << 8 | r;
    }

    pPaletteIDs[freeIdx] = uPaletteID;
    CalcPalettes_LUT(freeIdx);
    return freeIdx;
}

// inlined
//----- (mm6c::0045C610) ---------------------------------------------------
void PaletteManager::SetMistColor(unsigned char r, unsigned char g,
                                  unsigned char b) {
    pPalette_mistColor[0] = r;
    pPalette_mistColor[1] = g;
    pPalette_mistColor[2] = b;
}

//----- (0048A614) --------------------------------------------------------
void PaletteManager::RecalculateAll() {
    CalcPalettes_LUT(0);

    for (uint i = 1; i < 50; ++i)
        if (pPaletteIDs[i])
            CalcPalettes_LUT(i);
}

//----- (0047BE67) --------------------------------------------------------
uint16_t *PaletteManager::Get(int paletteIdx) {
    return &pPaletteManager->field_199600_palettes[paletteIdx][0][0];
}

// new
uint8_t *PaletteManager::GetBasePalette(int paletteID) {
    return &pPaletteManager->pBaseColors[paletteID][0][0];
}

int PaletteManager::GetPaletteIndex(int uPaletteID) {
    for (int i = 0; i < 50; ++i)
        if (pPaletteIDs[i] == uPaletteID)
            return i;

    logger->Warning("Palette %u not loaded. Returning Index to greyscale!", uPaletteID);
    return 0;
}

//----- (0047BE72) --------------------------------------------------------
uint16_t *PaletteManager::Get_Mist_or_Red_LUT(int paletteIdx, int a2, char a3) {
    int v3;  // eax@4

    if (a3 & 2 || engine->config->graphics.SoftwareModeRules.Get() && engine->config->graphics.AlternativePaletteMode.Get())
        v3 = 32 * paletteIdx + a2 + 3275;
    else
        v3 = 32 * paletteIdx + a2 + 1675;
    return (uint16_t *)((char *)&pPaletteManager + 512 * v3);
}
// 4D864C: using guessed type char _4D864C_force_sw_render_rules;

//----- (0041F50D) --------------------------------------------------------
uint16_t *PaletteManager::Get_Dark_or_Red_LUT(int paletteIdx, int a2, char a3) {
    int v3;  // eax@4

    if (a3 & 2 || engine->config->graphics.SoftwareModeRules.Get() && engine->config->graphics.AlternativePaletteMode.Get())
        v3 = 32 * paletteIdx + a2 + 3275;
    else
        v3 = 32 * paletteIdx + a2 + 75;
    return (uint16_t *)((char *)&pPaletteManager + 512 * v3);
}
// 4D864C: using guessed type char _4D864C_force_sw_render_rules;

//----- (0047C30E) --------------------------------------------------------
uint16_t *PaletteManager::_47C30E_get_palette(int paletteIdx, char a2) {
    char *result;  // eax@4

    if (a2 & 2 || engine->config->graphics.SoftwareModeRules.Get() && engine->config->graphics.AlternativePaletteMode.Get())
        result = (char *)pPaletteManager->field_199600_palettes[paletteIdx];
    else
        result = (char *)pPaletteManager->field_D1600[paletteIdx];
    return (uint16_t *)result;
}

//----- (0047C33F) --------------------------------------------------------
uint16_t *PaletteManager::_47C33F_get_palette(int paletteIdx, char a2) {
    uint16_t *result;  // eax@4

    if (a2 & 2 || engine->config->graphics.SoftwareModeRules.Get() && engine->config->graphics.AlternativePaletteMode.Get())
        result = (uint16_t *)pPaletteManager->field_199600_palettes[paletteIdx];
    else
        result = (uint16_t *)pPaletteManager->pPalette1[paletteIdx];
    return result;
}

//----- (0048A959) --------------------------------------------------------
int ReplaceHSV(unsigned int uColor, float h_replace, float s_replace, float v_replace) {
    float r = ((uColor & 0x00FF0000) >> 16) / 255.0f,
          g = ((uColor & 0x0000FF00) >> 8) / 255.0f,
          b = (uColor & 0x000000FF) / 255.0f;

    float h, s, v;
    RGB2HSV(r, g, b, &h, &s, &v);

    if (h_replace != -1.0) h = h_replace;
    if (s_replace != -1.0) s = s_replace;
    if (v_replace != -1.0) v = v_replace;
    HSV2RGB(&r, &g, &b, h, s, v);

    return (((uint)round_to_int(r * 255.0f) & 0xFF) << 16) |
           (((uint)round_to_int(g * 255.0f) & 0xFF) << 8) |
           (((uint)round_to_int(b * 255.0f) & 0xFF));
}
