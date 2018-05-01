#include <cstdlib>

#include "Engine/Engine.h"

#include "Engine/Objects/ObjectList.h"

#include "Engine/Graphics/Sprites.h"

#include "Engine/Tables/FrameTableInc.h"

//----- (0042EB42) --------------------------------------------------------
__int16 ObjectList::ObjectIDByItemID(unsigned __int16 uItemID) {
    unsigned int v2;  // edx@1
    signed int v3;    // eax@1
    char *v4;         // ecx@2

    v2 = this->uNumObjects;
    v3 = 0;
    if ((signed int)this->uNumObjects <= 0) {
    LABEL_5:
        v3 = 0;
    } else {
        v4 = (char *)&this->pObjects->uObjectID;
        while (uItemID != *(short *)v4) {
            ++v3;
            v4 += 56;
            if (v3 >= (signed int)v2) goto LABEL_5;
        }
    }
    return v3;
}
//----- (00459064) --------------------------------------------------------
void ObjectList::InitializeSprites() {
    for (uint i = 0; i < uNumObjects; ++i)
        pSpriteFrameTable->InitializeSprite(pObjects[i].uSpriteID);
}

//----- (00459090) --------------------------------------------------------
void ObjectList::ToFile() {
    ObjectList *v1;  // esi@1
    FILE *v2;        // eax@1
    FILE *v3;        // edi@1

    v1 = this;
    v2 = fopen("data\\dobjlist.bin", "wb");
    v3 = v2;
    if (!v2) Error("Unable to save dobjlist.bin!");
    fwrite(v1, 4u, 1u, v2);
    fwrite(v1->pObjects, 0x38u, v1->uNumObjects, v3);
    fclose(v3);
}

//----- (004590DC) --------------------------------------------------------
void ObjectList::FromFile(void *data_mm6, void *data_mm7, void *data_mm8) {
    uint num_mm6_objs = data_mm6 ? *(int *)data_mm6 : 0,
         num_mm7_objs = data_mm7 ? *(int *)data_mm7 : 0,
         num_mm8_objs = data_mm8 ? *(int *)data_mm8 : 0;

    uNumObjects = num_mm6_objs + num_mm7_objs + num_mm8_objs;
    Assert(uNumObjects);
    Assert(!num_mm8_objs);

    pObjects = (ObjectDesc *)malloc(uNumObjects * sizeof(ObjectDesc));
    memcpy(pObjects, (char *)data_mm7 + 4, num_mm7_objs * sizeof(ObjectDesc));
    for (uint i = 0; i < num_mm6_objs; ++i) {
        auto src = (ObjectDesc_mm6 *)((char *)data_mm6 + 4) + i;
        ObjectDesc *dst = &pObjects[num_mm7_objs + i];
        memcpy(dst->field_0, src->field_0, sizeof(dst->field_0));
        dst->uObjectID = src->uObjectID;
        dst->uRadius = src->uRadius;
        dst->uHeight = src->uHeight;
        dst->uFlags = src->uFlags;
        dst->uSpriteID = src->uSpriteID;
        dst->uLifetime = src->uLifetime;
        dst->uParticleTrailColor = src->uParticleTrailColor;
        dst->uSpeed = src->uSpeed;
        dst->uParticleTrailColorR = src->uParticleTrailColorR;
        dst->uParticleTrailColorG = src->uParticleTrailColorG;
        dst->uParticleTrailColorB = src->uParticleTrailColorB;
        dst->field_35_clr = src->field_35_clr;
        dst->field_36_clr = 0;
        dst->field_37_clr = 0;
    }
}

//----- (00459123) --------------------------------------------------------
bool ObjectList::FromFileTxt(const char *Args) {
    ObjectList *v2;         // ebx@1
    __int32 v3;             // edi@1
    FILE *v4;               // eax@1
    unsigned int v5;        // esi@3
    void *v6;               // eax@9
    FILE *v7;               // ST0C_4@11
    char *i;                // eax@11
    unsigned __int16 v9;    // ax@14
    const char *v10;        // ST20_4@14
    __int16 v11;            // ax@14
    const char *v12;        // ST1C_4@14
    __int16 v13;            // ax@14
    const char *v14;        // ST18_4@14
    __int16 v15;            // ax@14
    const char *v16;        // ST14_4@14
    __int16 v17;            // ax@14
    const char *v18;        // ST10_4@14
    __int16 v19;            // ax@14
    const char *v20;        // ST0C_4@14
    int v21;                // esi@16
    const char *v22;        // edi@16
    int v23;                // eax@17
    int v24;                // eax@19
    int v25;                // eax@21
    int v26;                // eax@21
    int v27;                // eax@21
    int v28;                // eax@23
    int v29;                // eax@25
    int v30;                // eax@27
    int v31;                // eax@29
    const char *v32;        // edi@30
    const char *v33;        // ST20_4@35
    int v34;                // eax@35
    char v35;               // al@35
    const char *v36;        // ST1C_4@35
    char v37;               // al@35
    const char *v38;        // ST18_4@35
    FrameTableTxtLine v40;  // [sp+8h] [bp-460h]@14
    FrameTableTxtLine v41;  // [sp+84h] [bp-3E4h]@12
    char Dest;              // [sp+100h] [bp-368h]@14
    char Buf;               // [sp+178h] [bp-2F0h]@3
    FrameTableTxtLine v44;  // [sp+36Ch] [bp-FCh]@4
    FrameTableTxtLine v45;  // [sp+3E8h] [bp-80h]@4
    FILE *File;             // [sp+464h] [bp-4h]@1
    unsigned int Argsa;     // [sp+470h] [bp+8h]@3
    int Argsb;              // [sp+470h] [bp+8h]@15

    v2 = this;
    free(this->pObjects);
    v3 = 0;
    v2->pObjects = nullptr;
    v2->uNumObjects = 0;
    v4 = fopen(Args, "r");
    File = v4;
    if (!v4) Error("ObjectDescriptionList::load - Unable to open file: %s.");

    v5 = 0;
    Argsa = 0;
    if (fgets(&Buf, 490, v4)) {
        do {
            *strchr(&Buf, 10) = 0;
            memcpy(&v45, frame_table_txt_parser(&Buf, &v44), sizeof(v45));
            if (v45.uPropCount && *v45.pProperties[0] != '/') ++Argsa;
        } while (fgets(&Buf, 490, File));
        v5 = Argsa;
        v3 = 0;
    }
    v2->uNumObjects = v5;
    v6 = malloc(sizeof(ObjectDesc) * v5);
    v2->pObjects = (ObjectDesc *)v6;
    if (v6 == (void *)v3) Error("ObjectDescriptionList::load - Out of Memory!");

    memset(v6, v3, sizeof(ObjectDesc) * v2->uNumObjects);
    v7 = File;
    v2->uNumObjects = v3;
    fseek(v7, v3, v3);
    for (i = fgets(&Buf, 490, File); i; i = fgets(&Buf, 490, File)) {
        *strchr(&Buf, 10) = 0;
        memcpy(&v45, frame_table_txt_parser(&Buf, &v41), sizeof(v45));
        if (v45.uPropCount && *v45.pProperties[0] != 47) {
            strcpy(v2->pObjects[v2->uNumObjects].field_0, v45.pProperties[0]);
            v9 = pSpriteFrameTable->FastFindSprite((char *)v45.pProperties[1]);
            v10 = v45.pProperties[2];
            v2->pObjects[v2->uNumObjects].uSpriteID = v9;
            v11 = atoi(v10);
            v12 = v45.pProperties[3];
            v2->pObjects[v2->uNumObjects].uObjectID = v11;
            v13 = atoi(v12);
            v14 = v45.pProperties[4];
            v2->pObjects[v2->uNumObjects].uRadius = v13;
            v15 = atoi(v14);
            v16 = v45.pProperties[5];
            v2->pObjects[v2->uNumObjects].uHeight = v15;
            v17 = atoi(v16);
            v18 = v45.pProperties[6];
            v2->pObjects[v2->uNumObjects].uLifetime = v17;
            v19 = atoi(v18);
            v20 = v45.pProperties[7];
            v2->pObjects[v2->uNumObjects].uSpeed = v19;
            strcpy(&Dest, v20);
            memcpy(&v44, frame_table_txt_parser(&Dest, &v40), sizeof(v44));
            if (v45.uPropCount > 7) {
                for (Argsb = 0; Argsb < v44.uPropCount; ++Argsb) {
                    v21 = Argsb;
                    v22 = v44.pProperties[Argsb];
                    if (!_stricmp(v44.pProperties[Argsb], "NoDraw")) {
                        v23 = (int)&v2->pObjects[v2->uNumObjects].uFlags;
                        *(char *)v23 |= 1u;
                    }
                    if (!_stricmp(v22, "Lifetime")) {
                        v24 = (int)&v2->pObjects[v2->uNumObjects].uFlags;
                        *(char *)v24 |= 4u;
                    }
                    if (!_stricmp(v22, "FTLifetime")) {
                        v25 = (int)&v2->pObjects[v2->uNumObjects];
                        *(short *)(v25 + 42) =
                            8 * pSpriteFrameTable
                                    ->pSpriteSFrames[*(short *)(v25 + 40)]
                                    .uAnimLength;
                        v26 = (int)&v2->pObjects[v2->uNumObjects].uFlags;
                        *(char *)v26 |= 8u;
                        v27 = (int)&v2->pObjects[v2->uNumObjects].uFlags;
                        *(char *)v27 |= 4u;
                    }
                    if (!_stricmp(v22, "NoPickup")) {
                        v28 = (int)&v2->pObjects[v2->uNumObjects].uFlags;
                        *(char *)v28 |= 0x10u;
                    }
                    if (!_stricmp(v22, "NoGravity")) {
                        v29 = (int)&v2->pObjects[v2->uNumObjects].uFlags;
                        *(char *)v29 |= 0x20u;
                    }
                    if (!_stricmp(v22, "FlagOnIntercept")) {
                        v30 = (int)&v2->pObjects[v2->uNumObjects].uFlags;
                        *(char *)v30 |= 0x40u;
                    }
                    if (!_stricmp(v22, "Bounce")) {
                        v31 = (int)&v2->pObjects[v2->uNumObjects].uFlags;
                        *(char *)v31 |= 0x80u;
                    }
                    v32 = v45.pProperties[v21];
                    if (!_stricmp(v45.pProperties[v21], "Fire"))
                        v2->pObjects[v2->uNumObjects].uFlags |= 0x200;
                    if (!_stricmp(v32, "Lines"))
                        v2->pObjects[v2->uNumObjects].uFlags |= 0x400;
                    if (!_stricmp(v44.pProperties[v21], "bits")) {
                        v33 = v44.pProperties[v21 + 1];
                        v34 = (int)&v2->pObjects[v2->uNumObjects].uFlags;
                        *(char *)(v34 + 1) |= 1u;
                        v35 = atoi(v33);
                        v36 = v44.pProperties[v21 + 2];
                        v2->pObjects[v2->uNumObjects].uParticleTrailColorR =
                            v35;
                        v37 = atoi(v36);
                        v38 = v44.pProperties[v21 + 3];
                        v2->pObjects[v2->uNumObjects].uParticleTrailColorG =
                            v37;
                        v2->pObjects[v2->uNumObjects].uParticleTrailColorB =
                            atoi(v38);
                    }
                }
            }
            ++v2->uNumObjects;
        }
    }
    fclose(File);
    return 1;
}
