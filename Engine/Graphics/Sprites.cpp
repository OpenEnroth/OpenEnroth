#include "Engine/Graphics/Sprites.h"

#include <cstdlib>
#include <cstring>
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

                        if (v8 == -1) {  // should we set hw_sprites as nullptr in these cases??
                            // __debugbreak();
                            logger->Warning(L"Sprite %S not loaded!", pSpriteSFrames[v3].texture_name.c_str());
                        }

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
                            if (v12 == -1) __debugbreak();
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
                                    sprintf(sprite_name, "%s4", Str);
                                    break;
                                case 3:
                                case 5:
                                    sprintf(sprite_name, "%s3", Str);
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
                            if (v12 == -1) __debugbreak();

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
                                // some names already passed through with codes attached
                                if (strlen(pSpriteSFrames[v3].texture_name.c_str()) < 7) {
                                    sprintf(sprite_name, "%s%i", pSpriteSFrames[v3].texture_name.c_str(), i);
                                } else {
                                    sprintf(sprite_name, "%s", pSpriteSFrames[v3].texture_name.c_str());
                                    // __debugbreak();
                                }
                            }

                            auto v12 = pSprites_LOD->LoadSprite(
                                sprite_name, pSpriteSFrames[v3].uPaletteID);
                            // pSpriteSFrames[v3].pHwSpriteIDs[i]=v12;

                            if (v12 == -1) __debugbreak();

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
int SpriteFrameTable::FastFindSprite(const char *pSpriteName) {
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
        int comparisonResult = _stricmp(pSpriteName, this->pSpritePFrames[middleFrameIndex]->icon_name.c_str());
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

void SpriteFrameTable::ToFile() {
    FILE *file = fopen("data\\dsft.bin", "wb");
    if (file == nullptr) {
        Error("Unable to save dsft.bin!");
    }
    fwrite(&uNumSpriteFrames, 4, 1, file);
    fwrite(&uNumEFrames, 4, 1, file);
    fwrite(pSpriteSFrames, 0x3C, uNumSpriteFrames, file);
    fwrite(pSpriteEFrames, 2, uNumEFrames, file);
    fclose(file);
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

    pSpritePFrames = (SpriteFrame **)malloc(sizeof(void *) * uNumSpriteFrames);

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

void _46E26D_collide_against_sprites(int a1, int a2) {
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
                                    v6 = pDecorationList->GetDecoration(v5->uDecorationDescID);
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


SpriteFrame *LevelDecorationChangeSeason(DecorationDesc *desc, int t, int month) {
    switch (month/*pParty->uCurrentMonth*/) {
        // case 531 (tree60), 536 (tree65), 537 (tree66) have no autumn/winter
        // sprites
        case 11:
        case 0:
        case 1:  // winter
        {
            switch (desc->uSpriteID) {
                // case 468:           //bush02    grows on swamps, which are
                // evergreeen actually
                case 548:  // flower10
                case 547:  // flower09
                case 541:  // flower03
                case 539:  // flower01
                    return nullptr;

                case 483:  // tree01
                case 486:  // tree04
                case 492:  // tree10
                {
                    pSpriteFrameTable->InitializeSprite(desc->uSpriteID + 2);
                    return pSpriteFrameTable->GetFrame(desc->uSpriteID + 2, t);
                }

                default:
                    return pSpriteFrameTable->GetFrame(desc->uSpriteID, t);
            }
        }

        case 2:
        case 3:
        case 4:  // spring
        {
            // switch (desc->uSpriteID) {}
            return pSpriteFrameTable->GetFrame(desc->uSpriteID, t);
        }

        case 8:
        case 9:
        case 10:  // autumn
        {
            switch (desc->uSpriteID) {
                // case 468: //bush02    grows on swamps, which are evergreeen
                // actually
                case 548:  // flower10
                case 547:  // flower09
                case 541:  // flower03
                case 539:  // flower01
                    return nullptr;

                case 483:  // tree01
                case 486:  // tree04
                case 492:  // tree10
                {
                    pSpriteFrameTable->InitializeSprite(desc->uSpriteID + 1);
                    return pSpriteFrameTable->GetFrame(desc->uSpriteID + 1, t);
                }

                default:
                    return pSpriteFrameTable->GetFrame(desc->uSpriteID, t);
            }
        } break;

        case 5:
        case 6:
        case 7:  // summer
                 // all green by default
        {
            return pSpriteFrameTable->GetFrame(desc->uSpriteID, t);
        }

        default:
            assert(/*pParty->uCurrentMonth*/month >= 0 && /*pParty->uCurrentMonth*/month < 12);
    }
}
