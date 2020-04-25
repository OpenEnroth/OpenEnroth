#include "Engine/Tables/PlayerFrameTable.h"

#include "Engine/Engine.h"

#include "Platform/Api.h"

#include "FrameTableInc.h"


//----- (00494AED) --------------------------------------------------------
unsigned int PlayerFrameTable::GetFrameIdByExpression(
    CHARACTER_EXPRESSION_ID expression) {
    for (uint i = 0; i < this->uNumFrames; i++) {
        if (this->pFrames[i].expression == expression) return i;
    }
    return 0;
}

//----- (00494B10) --------------------------------------------------------
PlayerFrame *PlayerFrameTable::GetFrameBy_x(unsigned int uFramesetID,
                                            unsigned int uFrameID) {
    unsigned int v3;      // esi@1
    __int16 v6;           // dx@2
    int v7;               // edx@3
    char *i;              // eax@3
    int v9;               // ecx@5
    PlayerFrame *result;  // eax@6

    v3 = uFramesetID;
    if (this->pFrames[uFramesetID].uFlags & 1 &&
        (v6 = this->pFrames[uFramesetID].uAnimLength) != 0) {
        v7 = ((signed int)uFrameID >> 3) % (unsigned __int16)v6;
        for (i = (char *)&this->pFrames[uFramesetID].uAnimTime;; i += 10) {
            v9 = *(short *)i;
            if (v7 <= v9) break;
            v7 -= v9;
            ++v3;
        }
        result = &this->pFrames[v3];
    } else {
        result = &this->pFrames[uFramesetID];
    }

    return result;
}

//----- (00494B5E) --------------------------------------------------------
PlayerFrame *PlayerFrameTable::GetFrameBy_y(int *pFramesetID, int *pAnimTime,
                                            int a4) {
    int v5;  // esi@1
    int v6;  // eax@2

    v5 = a4 + *pAnimTime;
    if (v5 < 8 * this->pFrames[*pFramesetID].uAnimTime) {
        *pAnimTime = v5;
    } else {
        v6 = rand() % 4 + 21;
        *pFramesetID = v6;
        *pAnimTime = 8 * v5 % this->pFrames[v6].uAnimTime;
    }
    return &this->pFrames[*pFramesetID];
}

//----- (00494BC3) --------------------------------------------------------
void PlayerFrameTable::ToFile() {
    PlayerFrameTable *v1;  // esi@1
    FILE *v2;              // eax@1
    FILE *v3;              // edi@1

    PlayerFrameTable *Str = this;

    v1 = Str;
    v2 = fcaseopen("data/dpft.bin", "wb");
    v3 = v2;
    if (!v2) Error("Unable to save dpft.bin");
    fwrite(v1, 4, 1, v2);
    fwrite(v1->pFrames, 0xAu, v1->uNumFrames, v3);
    fclose(v3);
}

//----- (00494C0F) --------------------------------------------------------
void PlayerFrameTable::FromFile(void *data_mm6, void *data_mm7,
                                void *data_mm8) {
    uint num_mm6_frames = data_mm6 ? *(int *)data_mm6 : 0,
         num_mm7_frames = data_mm7 ? *(int *)data_mm7 : 0,
         num_mm8_frames = data_mm8 ? *(int *)data_mm8 : 0;
    uNumFrames = num_mm6_frames + num_mm7_frames + num_mm8_frames;
    assert(uNumFrames);
    assert(!num_mm8_frames);

    pFrames = (PlayerFrame *)malloc(uNumFrames * sizeof(PlayerFrame));
    memcpy(pFrames, (char *)data_mm7 + 4, num_mm7_frames * sizeof(PlayerFrame));
    memcpy(pFrames + num_mm7_frames, (char *)data_mm6 + 4,
           num_mm6_frames * sizeof(PlayerFrame));
    memcpy(pFrames + num_mm6_frames + num_mm7_frames, (char *)data_mm8 + 4,
           num_mm8_frames * sizeof(PlayerFrame));
}

//----- (00494C5A) --------------------------------------------------------
//    int PlayerFrameTable::FromFileTxt(const char *Args) {
//        // PlayerFrameTable *v2; // ebx@1
//        FILE *v3;  // eax@1
//        int v4;    // esi@3
//        void *v5;  // eax@10
//        FILE *v6;  // ST0C_4@12
//        char *i;   // eax@12
//        //  __int16 v8; // ax@15
//        //  const char *v9; // ST10_4@15
//        //  unsigned __int16 v10; // ax@15
//        //  const char *v11; // ST0C_4@15
//        int j;  // esi@15
//        //  int v13; // eax@17
//        int v14;       // edx@22
//        int v15;       // ecx@23
//        int v16;       // eax@24
//        signed int k;  // eax@27
//        // PlayerFrame *v18; // edx@28
//        int v19;                // esi@28
//        int l;                  // ecx@29
//        char Buf;               // [sp+Ch] [bp-2F8h]@3
//        FrameTableTxtLine v23;  // [sp+200h] [bp-104h]@4
//        FrameTableTxtLine v24;  // [sp+27Ch] [bp-88h]@4
//        int v25;                // [sp+2F8h] [bp-Ch]@3
//        int v26;                // [sp+2FCh] [bp-8h]@3
//        FILE *File;             // [sp+300h] [bp-4h]@1
//        int Argsa;              // [sp+30Ch] [bp+8h]@28
//
//        __debugbreak();  // Ritor1;
//        // TileTable::dtor((TileTable *)this);
//        v3 = fcaseopen(Args, "r");
//        File = v3;
//        if (!v3) Error("PlayerFrameTable::load - Unable to open file: %s.", Args);
//        v4 = 0;
//        v25 = 0;
//        v26 = 1;
//        if (fgets(&Buf, 490, v3)) {
//            do {
//                *strchr(&Buf, 10) = 0;
//                memcpy(&v24, txt_file_frametable_parser(&Buf, &v23), sizeof(v24));
//                if (v24.uPropCount && *v24.pProperties[0] != 47) {
//                    if (v24.uPropCount < 3)
//                        Error(
//                            "PlayerFrameTable::load, too few arguments, %s line "
//                            "%i.",
//                            Args, v26);
//                    ++v25;
//                }
//                ++v26;
//            } while (fgets(&Buf, 490, File));
//            v4 = v25;
//        }
//        this->uNumFrames = v4;
//        v5 = malloc(sizeof(PlayerFrame) * v4);
//        this->pFrames = (PlayerFrame *)v5;
//        if (!v5) Error("PlayerFrameTable::load - Out of Memory!");
//        v6 = File;
//        this->uNumFrames = 0;
//        fseek(v6, 0, 0);
//        for (i = fgets(&Buf, 490, File); i; i = fgets(&Buf, 490, File)) {
//            *strchr(&Buf, 10) = 0;
//            memcpy(&v24, txt_file_frametable_parser(&Buf, &v23), sizeof(v24));
//            if (v24.uPropCount && *v24.pProperties[0] != 47) {
//                // v8 = atoi(v24.pProperties[0]);
//                // v9 = v24.pProperties[1];
//                this->pFrames[this->uNumFrames].expression =
//                    (CHARACTER_EXPRESSION_ID)atoi(v24.pProperties[0]);
//                // v10 = atoi(v9);
//                // v11 = v24.pProperties[2];
//                this->pFrames[this->uNumFrames].uTextureID =
//                    atoi(v24.pProperties[1]);
//                this->pFrames[this->uNumFrames].uAnimTime =
//                    atoi(v24.pProperties[2]);
//                this->pFrames[this->uNumFrames].uAnimLength = 0;
//                this->pFrames[this->uNumFrames].uFlags = 0;
//                for (j = 3; j < v24.uPropCount; ++j) {
//                    if (!_stricmp(v24.pProperties[j], "New"))
//                        this->pFrames[this->uNumFrames].uFlags |= 4;
//                }
//                ++this->uNumFrames;
//            }
//        }
//        fclose(File);
//
//        if ((signed int)(this->uNumFrames - 1) > 0) {
//            v15 = 0;
//            for (v14 = 0; v14 < this->uNumFrames - 1; ++v14) {
//                v16 = (int)&this->pFrames[v15];
//                if (!(*(char *)(v16 + 18) & 4)) this->pFrames[v14].uFlags |= 1;
//                ++v15;
//            }
//        }
//        for (k = 0; k < (signed int)this->uNumFrames; *(short *)(Argsa + 6) = v19) {
//            // v18 = this->pFrames;
//            Argsa = (int)&this->pFrames[k];
//            v19 = *(short *)(Argsa + 4);
//            if (this->pFrames[k].uFlags & 1) {
//                ++k;
//                for (l = (int)&this->pFrames[k]; this->pFrames[k].uFlags & 1;
//                     l += 10) {
//                    v19 += *(short *)(l + 4);
//                    ++k;
//                }
//                HEXRAYS_LOWORD(v19) = this->pFrames[k].uAnimTime + v19;
//            }
//            ++k;
//        }
//        return 1;
//    }
