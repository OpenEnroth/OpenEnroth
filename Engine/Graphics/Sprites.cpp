#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include <algorithm>

#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/OurMath.h"

#include "Engine/Objects/Actor.h"

#include "Engine/Tables/FrameTableInc.h"

#include "Engine/Serialization/LegacyImages.h"

#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/Sprites.h"

struct SpriteFrameTable *pSpriteFrameTable;

//----- (0044D4D8) --------------------------------------------------------
void SpriteFrameTable::ReleaseSFrames() {
    free(this->pSpriteSFrames);
    this->pSpriteSFrames = nullptr;
    this->uNumSpriteFrames = 0;
}

//----- (0044D4F6) --------------------------------------------------------
void SpriteFrameTable::ResetSomeSpriteFlags() {
    int v1;        // esi@1
    signed int i;  // edx@1
    char *v3;      // eax@2

    v1 = 0;
    for (i = 0; i < (signed int)this->uNumSpriteFrames; ++i) {
        v3 = (char *)&this->pSpriteSFrames[v1].uFlags;
        ++v1;
        *v3 &= 0x7Fu;
    }
}

//----- (0044D513) --------------------------------------------------------
void SpriteFrameTable::InitializeSprite(signed int uSpriteID) {
    // SpriteFrameTable *v2; // esi@1
    unsigned int v3;  // ebx@3
    // char *v4; // edx@3
    // int v5; // eax@3
    //    SpriteFrame *v6; // ecx@5
    //    int v7; // eax@5
    // __int16 v8; // ax@6
    // signed int v9; // edx@6
    // int v10; // ecx@6
    //    signed int v11; // edi@10
    // __int16 v12; // ax@16
    //    int v13; // ecx@16
    // size_t v14; // eax@19
    //    signed int v15; // edi@19
    //    __int16 v16; // ax@27
    //    int v17; // ecx@27
    //    signed int v18; // edi@29
    //    SpriteFrame *v19; // eax@30
    //    __int16 v20; // ax@45
    //    int v21; // ecx@45

    char Str[32];          // [sp+Ch] [bp-3Ch]@19
    char sprite_name[20];  // [sp+2Ch] [bp-1Ch]@15
    char Source[4];        // [sp+40h] [bp-8h]@19

    // v2 = this;
    if (uSpriteID <= this->uNumSpriteFrames) {
        if (uSpriteID >= 0) {
            v3 = uSpriteID;

            int uFlags = pSpriteSFrames[v3].uFlags;
            if (!(uFlags & 0x0080)) {  // not loaded
                pSpriteSFrames[v3].uFlags |= 0x80;  // set loaded
                while (1) {
                    pSpriteSFrames[v3].uPaletteIndex =
                        pPaletteManager->LoadPalette(
                            pSpriteSFrames[v3].uPaletteID);
                    if (uFlags & 0x10) {  // single frame per frame sequence
                        auto v8 = pSprites_LOD->LoadSprite(
                            pSpriteSFrames[v3].texture_name.c_str(),
                            pSpriteSFrames[v3].uPaletteID);
                        for (uint i = 0; i < 8; ++i) {
                            // pSpriteSFrames[v3].pHwSpriteIDs[i] = v8;
                            pSpriteSFrames[v3].hw_sprites[i] =
                                &pSprites_LOD->pHardwareSprites[v8];
                        }

                    } else if (uFlags & 0x10000) {
                        for (uint i = 0; i < 8; ++i) {
                            switch (i) {
                                case 3:
                                case 4:
                                case 5:
                                    sprintf(sprite_name, "%s4",
                                            pSpriteSFrames[v3]
                                                .texture_name.c_str());
                                    break;
                                case 2:
                                case 6:
                                    sprintf(sprite_name, "%s2",
                                            pSpriteSFrames[v3]
                                                .texture_name.c_str());
                                    break;
                                case 0:
                                case 1:
                                case 7:
                                    sprintf(sprite_name, "%s0",
                                            pSpriteSFrames[v3]
                                                .texture_name.c_str());
                                    break;
                            }
                            auto v12 = pSprites_LOD->LoadSprite(
                                sprite_name, pSpriteSFrames[v3].uPaletteID);
                            // pSpriteSFrames[v3].pHwSpriteIDs[i]=v12;
                            pSpriteSFrames[v3].hw_sprites[i] =
                                &pSprites_LOD->pHardwareSprites[v12];
                        }

                    } else if (uFlags & 0x40) {  // part of monster fidgeting seq
                        strcpy(Source, "stA");
                        strcpy(Str, pSpriteSFrames[v3].texture_name.c_str());
                        auto v14 = strlen(Str);
                        strcpy(&Str[v14 - 3], Source);
                        for (uint i = 0; i < 8; ++i) {
                            switch (i) {
                                case 0:
                                    sprintf(sprite_name, "%s0",
                                            pSpriteSFrames[v3]
                                                .texture_name.c_str());
                                    break;
                                case 4:
                                    sprintf(sprite_name, "%s4", &Str);
                                    break;
                                case 3:
                                case 5:
                                    sprintf(sprite_name, "%s3", &Str);
                                    break;
                                case 2:
                                case 6:
                                    sprintf(sprite_name, "%s2",
                                            pSpriteSFrames[v3]
                                                .texture_name.c_str());
                                    break;
                                case 1:
                                case 7:
                                    sprintf(sprite_name, "%s1",
                                            pSpriteSFrames[v3]
                                                .texture_name.c_str());
                                    break;
                            }
                            auto v12 = pSprites_LOD->LoadSprite(
                                sprite_name, pSpriteSFrames[v3].uPaletteID);
                            // pSpriteSFrames[v3].pHwSpriteIDs[i]=v12;
                            pSpriteSFrames[v3].hw_sprites[i] =
                                &pSprites_LOD->pHardwareSprites[v12];
                        }
                    } else {
                        for (uint i = 0; i < 8; ++i) {
                            if (((0x0100 << i) &
                                 pSpriteSFrames[v3].uFlags)) {  // mirrors
                                switch (i) {
                                    case 1:
                                        sprintf(sprite_name, "%s7",
                                                pSpriteSFrames[v3]
                                                    .texture_name.c_str());
                                        break;
                                    case 2:
                                        sprintf(sprite_name, "%s6",
                                                pSpriteSFrames[v3]
                                                    .texture_name.c_str());
                                        break;
                                    case 3:
                                        sprintf(sprite_name, "%s5",
                                                pSpriteSFrames[v3]
                                                    .texture_name.c_str());
                                        break;
                                    case 4:
                                        sprintf(sprite_name, "%s4",
                                                pSpriteSFrames[v3]
                                                    .texture_name.c_str());
                                        break;
                                    case 5:
                                        sprintf(sprite_name, "%s3",
                                                pSpriteSFrames[v3]
                                                    .texture_name.c_str());
                                        break;
                                    case 6:
                                        sprintf(sprite_name, "%s2",
                                                pSpriteSFrames[v3]
                                                    .texture_name.c_str());
                                        break;
                                    case 7:
                                        sprintf(sprite_name, "%s1",
                                                pSpriteSFrames[v3]
                                                    .texture_name.c_str());
                                        break;
                                }
                            } else {
                                sprintf(sprite_name, "%s%i",
                                        pSpriteSFrames[v3].texture_name.c_str(),
                                        i);
                            }
                            auto v12 = pSprites_LOD->LoadSprite(
                                sprite_name, pSpriteSFrames[v3].uPaletteID);
                            // pSpriteSFrames[v3].pHwSpriteIDs[i]=v12;
                            pSpriteSFrames[v3].hw_sprites[i] =
                                &pSprites_LOD->pHardwareSprites[v12];
                        }
                    }

                    if (!(pSpriteSFrames[v3].uFlags & 1)) return;
                    ++v3;
                }
            }
        }
    }
}

//----- (0044D813) --------------------------------------------------------
int SpriteFrameTable::FastFindSprite(char *pSpriteName) {
    signed int result;  // eax@2

    int searchResult = BinarySearch(pSpriteName);
    if (searchResult < 0)
        result = 0;
    else
        result = this->pSpriteEFrames[searchResult];
    return result;
}

//----- (0044D83A) --------------------------------------------------------
int SpriteFrameTable::BinarySearch(const char *pSpriteName) {
    int startPos = 0;
    int endPos = uNumEFrames;
    while (1) {
        int searchRange = endPos - startPos;
        int middleFrameIndex = startPos + (endPos - startPos) / 2;
        int comparisonResult =
            _stricmp(pSpriteName,
                     this->pSpritePFrames[middleFrameIndex]->icon_name.c_str());
        if (!comparisonResult) {
            return middleFrameIndex;
        }
        if (startPos == endPos) {
            return -1;
        }
        if (comparisonResult >= 0) {
            startPos += max(((endPos - startPos) / 2), 1);
        } else {
            endPos = max(((endPos - startPos) / 2), 1) + startPos;
        }
    }
}

//----- (0044D8D0) --------------------------------------------------------
SpriteFrame *SpriteFrameTable::GetFrame(unsigned int uSpriteID,
                                        unsigned int uTime) {
    SpriteFrame *v4;  // ecx@1

    v4 = &pSpriteSFrames[uSpriteID];
    if (~v4->uFlags & 1 || !v4->uAnimLength) return pSpriteSFrames + uSpriteID;

    for (uint t = (uTime / 8) % v4->uAnimLength; t > v4->uAnimTime; ++v4)
        t -= v4->uAnimTime;
    return v4;

    /*for (v4; v4->uAnimTime <= t; ++v4)

      v6 = (uTime / 8) % v4->uAnimLength;
      //v7 = uSpriteID;
      for ( i = (char *)&v4->uAnimTime; ; i += 60 )
      {
        v9 = *(short *)i;
        if ( v6 <= v9 )
          break;
        v6 -= v9;
        ++v7;
      }
      return &pSpriteSFrames[v7];*/
}

//----- (0044D91F) --------------------------------------------------------
SpriteFrame *SpriteFrameTable::GetFrameBy_x(unsigned int uSpriteID,
                                            signed int a3) {
    SpriteFrame *v3;      // edi@1
    SpriteFrame *v4;      // esi@1
    __int16 v5;           // ax@2
    int v6;               // ecx@3
    int v7;               // edx@3
    unsigned int v8;      // eax@3
    int v9;               // ecx@3
    char *i;              // edx@3
    int v11;              // esi@5
    SpriteFrame *result;  // eax@6

    v3 = this->pSpriteSFrames;
    v4 = &v3[uSpriteID];
    if (v4->uFlags & 1 && (v5 = v4->uAnimLength) != 0) {
        v6 = v5;
        v7 = a3 % v5;
        v8 = uSpriteID;
        v9 = v6 - v7;
        for (i = (char *)&v4->uAnimTime;; i += 60) {
            v11 = *(short *)i;
            if (v9 <= v11) break;
            v9 -= v11;
            ++v8;
        }
        result = &v3[v8];
    } else {
        result = &v3[uSpriteID];
    }
    return result;
}

//----- (0044D96D) --------------------------------------------------------
void SpriteFrameTable::ToFile() {
    SpriteFrameTable *v1;  // esi@1
    FILE *v2;              // eax@1
    FILE *v3;              // edi@1

    v1 = this;
    v2 = fopen("data\\dsft.bin", "wb");
    v3 = v2;
    if (!v2) Error("Unable to save dsft.bin!");
    fwrite(v1, 4u, 1u, v2);
    fwrite(&v1->uNumEFrames, 4u, 1u, v3);
    fwrite(v1->pSpriteSFrames, 0x3Cu, v1->uNumSpriteFrames, v3);
    fwrite(v1->pSpriteEFrames, 2u, v1->uNumEFrames, v3);
    fclose(v3);
}

bool SpriteFrame::Deserialize(const struct SpriteFrame_MM7 *data) {
    if (data) {
        this->icon_name = data->pIconName;
        std::transform(this->icon_name.begin(), this->icon_name.end(),
                       this->icon_name.begin(), ::tolower);

        this->texture_name = data->pTextureName;
        std::transform(this->texture_name.begin(), this->texture_name.end(),
                       this->texture_name.begin(), ::tolower);

        for (unsigned int i = 0; i < 8; ++i) {
            this->hw_sprites[i] = nullptr;
        }

        this->scale = fixed(data->scale);
        this->uFlags = data->uFlags;

        this->uGlowRadius = data->uGlowRadius;
        this->uPaletteID = data->uPaletteID;
        this->uPaletteIndex = data->uPaletteIndex;
        this->uAnimTime = data->uAnimTime;
        this->uAnimLength = data->uAnimLength;

        return true;
    }
    return false;
}

//----- (0044D9D7) --------------------------------------------------------
void SpriteFrameTable::FromFile(void *data_mm6, void *data_mm7,
                                void *data_mm8) {
    uint num_mm6_frames = 0;
    uint num_mm6_eframes = 0;
    if (data_mm6) {
        num_mm6_frames = *(int *)data_mm6;
        num_mm6_eframes = *((int *)data_mm6 + 1);
    }

    uint num_mm7_frames = 0;
    uint num_mm7_eframes = 0;
    if (data_mm7) {
        num_mm7_frames = *(int *)data_mm7;
        num_mm7_eframes = *((int *)data_mm7 + 1);
    }

    uint num_mm8_frames = 0;
    uint num_mm8_eframes = 0;
    if (data_mm8) {
        num_mm8_frames = *(int *)data_mm8;
        num_mm8_eframes = *((int *)data_mm8 + 1);
    }

    this->uNumSpriteFrames =
        num_mm7_frames /*+ num_mm6_frames + num_mm8_frames*/;

    this->pSpriteSFrames = new SpriteFrame[this->uNumSpriteFrames];
    for (unsigned int i = 0; i < this->uNumSpriteFrames; ++i) {
        auto res = this->pSpriteSFrames[i].Deserialize(
            (SpriteFrame_MM7 *)((char *)data_mm7 + 8) + i);

        if (!res) {
            logger->Warning(L"MM7 Sprite %u deserialization failed", i);
        }
    }

    this->uNumEFrames = num_mm7_eframes /*+ num_mm6_eframes + num_mm8_eframes*/;
    this->pSpriteEFrames = (__int16 *)malloc(uNumSpriteFrames * sizeof(short));

    uint mm7_frames_size = num_mm7_frames * sizeof(SpriteFrame_MM7);
    memcpy(pSpriteEFrames, (char *)data_mm7 + 8 + mm7_frames_size,
           2 * num_mm7_eframes);

    pSpritePFrames = (SpriteFrame **)malloc(4 * uNumSpriteFrames);

    /*uint mm6_frames_size = num_mm6_frames * sizeof(SpriteFrame_mm6);
    for (uint i = 0; i < num_mm6_frames; ++i)
    {
        memcpy(pSpriteSFrames + num_mm7_frames + i, (char *)data_mm6 + 8 + i *
    sizeof(SpriteFrame_mm6), sizeof(SpriteFrame_mm6));
        pSpriteSFrames[num_mm7_frames + i].uAnimLength = 0;
    }
    memcpy(pSpriteEFrames + num_mm7_frames, (char *)data_mm6 + 8 +
    mm6_frames_size, 2 * num_mm6_eframes);*/

    /*uint mm8_frames_size = num_mm8_frames * sizeof(SpriteFrame);
    memcpy(pSpriteSFrames + num_mm6_frames + num_mm7_frames, (char *)data_mm8 +
    8, mm8_frames_size); memcpy(pSpriteEFrames + num_mm6_frames +
    num_mm7_frames, (char *)data_mm8 + 8 + mm8_frames_size, 2 *
    num_mm8_eframes);*/

    // the original was using num_mmx_frames, but never accessed any element
    // beyond num_mmx_eframes, but boing beyong eframes caused invalid memory
    // accesses
    for (uint i = 0;
         i < num_mm7_eframes /*+ num_mm6_eframes + num_mm8_eframes*/; ++i)
        pSpritePFrames[i] = &pSpriteSFrames[pSpriteEFrames[i]];
}

//----- (0044DA92) --------------------------------------------------------
bool SpriteFrameTable::FromFileTxt(const char *Args) {
    SpriteFrameTable *v2;   // ebx@1
    FILE *v3;               // eax@1
    unsigned int v4;        // esi@3
    signed int result;      // eax@10
    FILE *v6;               // ST18_4@11
    char *i;                // eax@11
    const char *v8;         // ST20_4@14
    __int16 v9;             // ax@14
    const char *v10;        // ST0C_4@14
    double v11;             // st7@14
    int v12;                // eax@14
    const char *v13;        // ST04_4@14
    __int16 v14;            // ax@14
    const char *v15;        // ST00_4@14
    int v16;                // eax@14
    int v17;                // eax@17
    int v18;                // eax@23
    int v19;                // eax@24
    int v20;                // eax@25
    int v21;                // eax@28
    int v22;                // eax@29
    int j;                  // edi@30
    const char *v24;        // esi@31
    int v25;                // eax@32
    int v26;                // edx@53
    int v27;                // ecx@54
    int v28;                // eax@55
    signed int k;           // edx@58
    SpriteFrame *v30;       // ecx@59
    int v31;                // esi@59
    int l;                  // eax@60
    signed int v33;         // eax@65
    int v34;                // edi@66
    int v35;                // esi@66
    SpriteFrame **v36;      // eax@69
    int v37;                // ecx@69
    SpriteFrame *v38;       // edx@69
    __int16 *v39;           // eax@69
    int v40;                // ecx@69
    char Buf[500];          // [sp+Ch] [bp-2F0h]@3
    FrameTableTxtLine v42;  // [sp+200h] [bp-FCh]@4
    FrameTableTxtLine v43;  // [sp+27Ch] [bp-80h]@4
    FILE *File;             // [sp+2F8h] [bp-4h]@1
    unsigned int Argsa;     // [sp+304h] [bp+8h]@3
    int Argsb;              // [sp+304h] [bp+8h]@59
    FILE *Argsc;            // [sp+304h] [bp+8h]@67

    v2 = this;
    ReleaseSFrames();
    v3 = fopen(Args, "r");
    File = v3;
    if (!v3) Error("CSpriteFrameTable::load - Unable to open file: %s.", Args);

    v4 = 0;
    Argsa = 0;
    if (fgets(Buf, 490, v3)) {
        do {
            *strchr(Buf, '\n') = 0;
            memcpy(&v43, frame_table_txt_parser(Buf, &v42), sizeof(v43));
            if (v43.uPropCount && *v43.pProperties[0] != '/') ++Argsa;
        } while (fgets(Buf, 490, File));
        v4 = Argsa;
    }
    v2->uNumSpriteFrames = v4;
    v2->pSpriteSFrames = (SpriteFrame *)malloc(60 * v4);
    v2->pSpriteEFrames = (__int16 *)malloc(2 * v2->uNumSpriteFrames);
    v2->pSpritePFrames = (SpriteFrame **)malloc(4 * v2->uNumSpriteFrames);
    if (v2->pSpriteSFrames) {
        v6 = File;
        v2->uNumSpriteFrames = 0;
        fseek(v6, 0, 0);
        for (i = fgets(Buf, 490, File); i; i = fgets(Buf, 490, File)) {
            *strchr(Buf, 10) = 0;
            memcpy(&v43, frame_table_txt_parser(Buf, &v42), sizeof(v43));
            if (v43.uPropCount && *v43.pProperties[0] != '/') {
                v8 = v43.pProperties[0];
                v2->pSpriteSFrames[v2->uNumSpriteFrames].uFlags = 0;
                v2->pSpriteSFrames[v2->uNumSpriteFrames].uPaletteIndex = 0;
                v2->pSpriteSFrames[v2->uNumSpriteFrames].icon_name = v8;
                v2->pSpriteSFrames[v2->uNumSpriteFrames].texture_name =
                    v43.pProperties[1];
                v9 = atoi(v43.pProperties[3]);
                v10 = v43.pProperties[4];
                v2->pSpriteSFrames[v2->uNumSpriteFrames].uPaletteID = v9;
                v11 = atof(v10) * 65536.0;
                v12 = abs((signed __int64)v11);
                v13 = v43.pProperties[5];
                v2->pSpriteSFrames[v2->uNumSpriteFrames].scale = fixed(v12);
                v14 = atoi(v13);
                v15 = v43.pProperties[6];
                v2->pSpriteSFrames[v2->uNumSpriteFrames].uGlowRadius = v14;
                v2->pSpriteSFrames[v2->uNumSpriteFrames].uAnimTime = atoi(v15);
                v16 = (int)&v2->pSpriteSFrames[v2->uNumSpriteFrames];
                if (*(short *)(v16 + 48)) *(int *)(v16 + 44) |= 2u;
                if (!_stricmp(v43.pProperties[2], "new")) {
                    v17 = (int)&v2->pSpriteSFrames[v2->uNumSpriteFrames].uFlags;
                    *(int *)v17 |= 4u;
                    v2->pSpritePFrames[v2->uNumEFrames] =
                        &v2->pSpriteSFrames[v2->uNumSpriteFrames];
                    v2->pSpriteEFrames[v2->uNumEFrames++] =
                        (short)v2->uNumSpriteFrames;
                }
                if (!_stricmp(v43.pProperties[10], "1"))
                    BYTE2(v2->pSpriteSFrames[v2->uNumSpriteFrames].uFlags) |=
                        4u;
                if (!_stricmp(v43.pProperties[11], "1"))
                    BYTE2(v2->pSpriteSFrames[v2->uNumSpriteFrames].uFlags) |=
                        2u;
                if (!_stricmp(v43.pProperties[12], "1")) {
                    v18 = (int)&v2->pSpriteSFrames[v2->uNumSpriteFrames].uFlags;
                    *(int *)v18 |= 0x20u;
                }
                v19 = atoi(v43.pProperties[7]) - 1;
                if (v19) {
                    v20 = v19 - 2;
                    if (v20) {
                        if (v20 == 2)
                            BYTE1(v2->pSpriteSFrames[v2->uNumSpriteFrames]
                                      .uFlags) |= 0xE0u;
                    } else {
                        v21 = (int)&v2->pSpriteSFrames[v2->uNumSpriteFrames]
                                  .uFlags;
                        *(int *)v21 |= 0x1E000u;
                    }
                } else {
                    v22 = (int)&v2->pSpriteSFrames[v2->uNumSpriteFrames].uFlags;
                    *(int *)v22 |= 0x10u;
                }
                for (j = 13; j < v43.uPropCount; ++j) {
                    v24 = v43.pProperties[j];
                    if (_stricmp(v43.pProperties[j], "Luminous")) {
                        if (_stricmp(v24, "Mirror0")) {
                            if (_stricmp(v24, "Mirror1")) {
                                if (_stricmp(v24, "Mirror2")) {
                                    if (_stricmp(v24, "Mirror3")) {
                                        if (_stricmp(v24, "Mirror4")) {
                                            if (_stricmp(v24, "Mirror5")) {
                                                if (_stricmp(v24, "Mirror6")) {
                                                    if (!_stricmp(v24,
                                                                  "Mirror7"))
                                                        BYTE1(
                                                            v2->pSpriteSFrames
                                                                [v2->uNumSpriteFrames]
                                                                    .uFlags) |=
                                                            0x80u;
                                                } else {
                                                    BYTE1(
                                                        v2->pSpriteSFrames
                                                            [v2->uNumSpriteFrames]
                                                                .uFlags) |=
                                                        0x40u;
                                                }
                                            } else {
                                                BYTE1(v2->pSpriteSFrames
                                                          [v2->uNumSpriteFrames]
                                                              .uFlags) |= 0x20u;
                                            }
                                        } else {
                                            BYTE1(v2->pSpriteSFrames
                                                      [v2->uNumSpriteFrames]
                                                          .uFlags) |= 0x10u;
                                        }
                                    } else {
                                        BYTE1(v2->pSpriteSFrames
                                                  [v2->uNumSpriteFrames]
                                                      .uFlags) |= 8u;
                                    }
                                } else {
                                    BYTE1(
                                        v2->pSpriteSFrames[v2->uNumSpriteFrames]
                                            .uFlags) |= 4u;
                                }
                            } else {
                                BYTE1(v2->pSpriteSFrames[v2->uNumSpriteFrames]
                                          .uFlags) |= 2u;
                            }
                        } else {
                            BYTE1(v2->pSpriteSFrames[v2->uNumSpriteFrames]
                                      .uFlags) |= 1u;
                        }
                    } else {
                        v25 = (int)&v2->pSpriteSFrames[v2->uNumSpriteFrames]
                                  .uFlags;
                        *(int *)v25 |= 2u;
                    }
                }
                ++v2->uNumSpriteFrames;
            }
        }
        fclose(File);
        v26 = 0;
        if ((signed int)(v2->uNumSpriteFrames - 1) > 0) {
            v27 = 0;
            do {
                v28 = (int)&v2->pSpriteSFrames[v27];
                if (!(*(char *)(v28 + 104) & 4)) *(int *)(v28 + 44) |= 1u;
                ++v26;
                ++v27;
            } while (v26 < (signed int)(v2->uNumSpriteFrames - 1));
        }
        for (k = 0; k < (signed int)v2->uNumSpriteFrames;
             *(short *)(Argsb + 56) = v31) {
            v30 = v2->pSpriteSFrames;
            Argsb = (int)&v30[k];
            v31 = *(short *)(Argsb + 54);
            if (*(char *)(Argsb + 44) & 1) {
                ++k;
                for (l = (int)&v30[k]; *(char *)(l + 44) & 1; l += 60) {
                    v31 += *(short *)(l + 54);
                    ++k;
                }
                HEXRAYS_LOWORD(v31) = v30[k].uAnimTime + v31;
            }
            ++k;
        }
        v33 = v2->uNumEFrames;
        if (v33 > 0) {
            v34 = 0;
            v35 = 0;
            File = (FILE *)1;
            do {
                Argsc = File;
                if ((signed int)File < v33) {
                    do {
                        if (_stricmp(
                                v2->pSpritePFrames[(int)Argsc]
                                    ->icon_name.c_str(),
                                v2->pSpritePFrames[v35]->icon_name.c_str()) <
                            0) {
                            v36 = v2->pSpritePFrames;
                            v37 = (int)&v36[(int)Argsc];
                            v38 = *(SpriteFrame **)v37;
                            *(int *)v37 = (int)v36[v35];
                            v2->pSpritePFrames[v35] = v38;
                            v39 = v2->pSpriteEFrames;
                            v40 = (int)&v39[(int)Argsc];
                            HEXRAYS_LOWORD(v38) = *(short *)v40;
                            *(short *)v40 = v39[v34];
                            v2->pSpriteEFrames[v34] = (signed __int16)v38;
                        }
                        Argsc = (FILE *)((char *)Argsc + 1);
                    } while ((signed int)Argsc < v2->uNumEFrames);
                }
                File = (FILE *)((char *)File + 1);
                v33 = v2->uNumEFrames;
                ++v35;
                ++v34;
            } while ((signed int)((char *)File - 1) < v33);
        }
        result = 1;
    } else {
        logger->Warning(L"CSpriteFrameTable::load - Out of Memory!");
        fclose(File);
        result = 0;
    }
    return result;
}
//----- (0046E26D) --------------------------------------------------------
void _46E26D_collide_against_sprites(signed int a1, signed int a2) {
    int v2;                 // edx@5
    unsigned __int16 *v3;   // eax@5
    unsigned __int16 v4;    // ax@6
    LevelDecoration *v5;    // edi@7
    DecorationDesc *v6;     // esi@8
    int v7;                 // edx@9
    int v8;                 // eax@9
    int v9;                 // ecx@11
    int v10;                // ebx@13
    int v11;                // esi@13
    int v12;                // ebp@15
    int v13;                // ebx@15
    int v14;                // esi@16
    int v15;                // edi@17
    int v16;                // eax@17
    int v17;                // esi@19
    char v18;               // zf@23
    int v19;                // [sp+0h] [bp-10h]@15
    unsigned __int16 *v20;  // [sp+4h] [bp-Ch]@5
    int v21;                // [sp+8h] [bp-8h]@15
    int v22;                // [sp+Ch] [bp-4h]@13

    if (a1 >= 0) {
        if (a1 <= 127) {
            if (a2 >= 0) {
                if (a2 <= 127) {
                    v2 = a1 + (a2 << 7);
                    v3 = &pOutdoor->pFaceIDLIST[pOutdoor->pOMAP[v2]];
                    v20 = &pOutdoor->pFaceIDLIST[pOutdoor->pOMAP[v2]];
                    if (v3) {
                        do {
                            v4 = *v3;
                            if (PID_TYPE(v4) == OBJECT_Decoration) {
                                v5 =
                                    &pLevelDecorations[(signed __int16)v4 >> 3];
                                if (!(v5->uFlags &
                                      LEVEL_DECORATION_INVISIBLE)) {
                                    v6 = &pDecorationList->pDecorations
                                              [v5->uDecorationDescID];
                                    if (!v6->CanMoveThrough()) {
                                        v7 = v6->uRadius;
                                        v8 = v5->vPosition.x;
                                        if (stru_721530.sMaxX <= v8 + v7) {
                                            if (stru_721530.sMinX >= v8 - v7) {
                                                v9 = v5->vPosition.y;
                                                if (stru_721530.sMaxY <=
                                                    v9 + v7) {
                                                    if (stru_721530.sMinY >=
                                                        v9 - v7) {
                                                        v10 =
                                                            v6->uDecorationHeight;
                                                        v11 = v5->vPosition.z;
                                                        v22 = v10;
                                                        if (stru_721530.sMaxZ <=
                                                            v11 + v10) {
                                                            if (stru_721530
                                                                    .sMinZ >=
                                                                v11) {
                                                                v12 =
                                                                    v8 -
                                                                    stru_721530
                                                                        .normal
                                                                        .x;
                                                                v19 =
                                                                    v9 -
                                                                    stru_721530
                                                                        .normal
                                                                        .y;
                                                                v13 =
                                                                    stru_721530
                                                                        .prolly_normal_d +
                                                                    v7;
                                                                v21 =
                                                                    ((v8 -
                                                                      stru_721530
                                                                          .normal
                                                                          .x) *
                                                                         stru_721530
                                                                             .direction
                                                                             .y -
                                                                     (v9 -
                                                                      stru_721530
                                                                          .normal
                                                                          .y) *
                                                                         stru_721530
                                                                             .direction
                                                                             .x) >>
                                                                    16;
                                                                if (abs(v21) <=
                                                                    stru_721530
                                                                            .prolly_normal_d +
                                                                        v7) {
                                                                    v14 =
                                                                        (v12 *
                                                                             stru_721530
                                                                                 .direction
                                                                                 .x +
                                                                         v19 *
                                                                             stru_721530
                                                                                 .direction
                                                                                 .y) >>
                                                                        16;
                                                                    if (v14 >
                                                                        0) {
                                                                        v15 =
                                                                            v5->vPosition
                                                                                .z;
                                                                        v16 =
                                                                            stru_721530
                                                                                .normal
                                                                                .z +
                                                                            fixpoint_mul(
                                                                                stru_721530
                                                                                    .direction
                                                                                    .z,
                                                                                v14);
                                                                        if (v16 >=
                                                                            v15) {
                                                                            if (v16 <=
                                                                                v22 +
                                                                                    v15) {
                                                                                v17 =
                                                                                    v14 -
                                                                                    integer_sqrt(
                                                                                        v13 *
                                                                                            v13 -
                                                                                        v21 *
                                                                                            v21);
                                                                                if (v17 <
                                                                                    0)
                                                                                    v17 =
                                                                                        0;
                                                                                if (v17 <
                                                                                    stru_721530
                                                                                        .field_7C) {
                                                                                    stru_721530
                                                                                        .field_7C =
                                                                                        v17;
                                                                                    stru_721530
                                                                                        .pid =
                                                                                        (signed __int16)*v20;
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            v3 = v20 + 1;
                            v18 = *v20 == 0;
                            ++v20;
                        } while (!v18);
                    }
                }
            }
        }
    }
}
