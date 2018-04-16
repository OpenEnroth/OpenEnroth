#include "Engine/MapInfo.h"

#include <stdlib.h>

#include "Engine/Engine.h"

#include "Engine/Graphics/Indoor.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "LOD.h"

#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "OurMath.h"
#include "Party.h"

//----- (00453F62) --------------------------------------------------------
void MapStats::Initialize() {
    char work_str[32];  // [sp+Ch] [bp-34h]@3
    int work_str_pos;
    int work_str_len;
    int i;
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;
    //  int item_counter;

    free(pMapStatsTXT_Raw);
    pMapStatsTXT_Raw = (char *)pEvents_LOD->LoadRaw("MapStats.txt", 0);
    strtok(pMapStatsTXT_Raw, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");

    for (i = 1; i < 77; ++i) {
        test_string = strtok(NULL, "\r") + 1;
        break_loop = false;
        decode_step = 0;
        do {
            c = *(unsigned char *)test_string;
            temp_str_len = 0;
            while ((c != '\t') && (c > 0)) {
                ++temp_str_len;
                c = test_string[temp_str_len];
            }
            tmp_pos = test_string + temp_str_len;
            if (*tmp_pos == 0) break_loop = true;
            *tmp_pos = 0;
            if (temp_str_len) {
                switch (decode_step) {
                    case 1:
                        pInfos[i].pName = RemoveQuotes(test_string);
                        break;
                    case 2:
                        pInfos[i].pFilename = RemoveQuotes(test_string);
                        break;
                    case 3:
                        pInfos[i].uNumResets = atoi(test_string);
                        break;
                    case 4:
                        pInfos[i].uFirstVisitedAt = atoi(test_string);
                        break;
                    case 5:
                        pInfos[i]._per = atoi(test_string);
                        break;
                    case 6:
                        pInfos[i].uRespawnIntervalDays = atoi(test_string);
                        break;
                    case 7:
                        pInfos[i]._alert_days = atoi(test_string);
                        break;
                    case 8:
                        pInfos[i]._steal_perm = atoi(test_string);
                        break;
                    case 9:
                        pInfos[i].LockX5 = atoi(test_string);
                        break;
                    case 10:
                        pInfos[i].Trap_D20 = atoi(test_string);
                        break;
                    case 11:
                        pInfos[i].Treasure_prob = atoi(test_string);
                        break;
                    case 12:
                        pInfos[i].Encounter_percent = atoi(test_string);
                        break;
                    case 13:
                        pInfos[i].EncM1percent = atoi(test_string);
                        break;
                    case 14:
                        pInfos[i].EncM2percent = atoi(test_string);
                        break;
                    case 15:
                        pInfos[i].EncM3percent = atoi(test_string);
                        break;
                    case 16:
                        pInfos[i].pEncounterMonster1Texture =
                            RemoveQuotes(test_string);
                        break;
                    case 18:
                        pInfos[i].Dif_M1 = atoi(test_string);
                        break;
                    case 19:
                        pInfos[i].uEncounterMonster1AtLeast = 1;
                        pInfos[i].uEncounterMonster1AtMost = 1;
                        strcpy(work_str, test_string);
                        work_str_pos = 0;
                        work_str_len = strlen(work_str);
                        if (work_str_len) {
                            while (work_str[work_str_pos] != '-') {
                                ++work_str_pos;
                                if (work_str_pos >= work_str_len) break;
                            }
                            work_str[work_str_pos] = 0;
                            pInfos[i].uEncounterMonster1AtLeast =
                                atoi(work_str);
                            if (work_str_pos < work_str_len)
                                pInfos[i].uEncounterMonster1AtMost =
                                    atoi(&work_str[work_str_pos + 1]);
                            else
                                pInfos[i].uEncounterMonster1AtMost =
                                    pInfos[i].uEncounterMonster1AtLeast;
                        }
                        break;
                    case 20:
                        pInfos[i].pEncounterMonster2Texture =
                            RemoveQuotes(test_string);
                        break;
                    case 22:
                        pInfos[i].Dif_M2 = atoi(test_string);
                        break;
                    case 23:
                        pInfos[i].uEncounterMonster2AtLeast = 1;
                        pInfos[i].uEncounterMonster2AtMost = 1;
                        strcpy(work_str, test_string);
                        work_str_pos = 0;
                        work_str_len = strlen(work_str);
                        if (work_str_len) {
                            while (work_str[work_str_pos] != '-') {
                                ++work_str_pos;
                                if (work_str_pos >= work_str_len) break;
                            }
                            work_str[work_str_pos] = 0;
                            pInfos[i].uEncounterMonster2AtLeast =
                                atoi(work_str);
                            if (work_str_pos < work_str_len)
                                pInfos[i].uEncounterMonster2AtMost =
                                    atoi(&work_str[work_str_pos + 1]);
                            else
                                pInfos[i].uEncounterMonster2AtMost =
                                    pInfos[i].uEncounterMonster2AtLeast;
                        }
                        break;
                    case 24:
                        pInfos[i].pEncounterMonster3Texture =
                            RemoveQuotes(test_string);
                        break;
                    case 26:
                        pInfos[i].Dif_M3 = atoi(test_string);
                        break;
                    case 27:
                        pInfos[i].uEncounterMonster3AtLeast = 1;
                        pInfos[i].uEncounterMonster3AtMost = 1;
                        strcpy(work_str, test_string);
                        work_str_pos = 0;
                        work_str_len = strlen(work_str);
                        if (work_str_len) {
                            while (work_str[work_str_pos] != '-') {
                                ++work_str_pos;
                                if (work_str_pos >= work_str_len) break;
                            }
                            work_str[work_str_pos] = 0;
                            pInfos[i].uEncounterMonster3AtLeast =
                                atoi(work_str);
                            if (work_str_pos < work_str_len)
                                pInfos[i].uEncounterMonster3AtMost =
                                    atoi(&work_str[work_str_pos + 1]);
                            else
                                pInfos[i].uEncounterMonster3AtMost =
                                    pInfos[i].uEncounterMonster3AtLeast;
                        }
                        break;
                    case 28:
                        pInfos[i].uRedbookTrackID = atoi(test_string);
                        break;
                    case 29: {
                        if (!strcmp(test_string, "CAVE")) {
                            pInfos[i].uEAXEnv = 8;
                            break;
                        }
                        if (!strcmp(test_string, "STONEROOM")) {
                            pInfos[i].uEAXEnv = 5;
                            break;
                        }
                        if (!strcmp(test_string, "MOUNTAINS")) {
                            pInfos[i].uEAXEnv = 17;
                            break;
                        }
                        if (!strcmp(test_string, "PLAIN")) {
                            pInfos[i].uEAXEnv = 19;
                            break;
                        }
                        if (!strcmp(test_string, "FOREST")) {
                            pInfos[i].uEAXEnv = 15;
                            break;
                        }
                        if (!strcmp(test_string, "CITY")) {
                            pInfos[i].uEAXEnv = 16;
                            break;
                        }
                        if (!strcmp(test_string, "UNDERWATER")) {
                            pInfos[i].uEAXEnv = 22;
                            break;
                        }
                        if (!strcmp(test_string, "ARENA")) {
                            pInfos[i].uEAXEnv = 9;
                            break;
                        }
                        if (!strcmp(test_string, "GENERIC")) {
                            pInfos[i].uEAXEnv = 0;
                            break;
                        }
                        if (!strcmp(test_string, "PADDEDCELL")) {
                            pInfos[i].uEAXEnv = 1;
                            break;
                        }
                        if (!strcmp(test_string, "ROOM")) {
                            pInfos[i].uEAXEnv = 2;
                            break;
                        }
                        if (!strcmp(test_string, "BATHROOM")) {
                            pInfos[i].uEAXEnv = 3;
                            break;
                        }
                        if (!strcmp(test_string, "LIVINGROOM")) {
                            pInfos[i].uEAXEnv = 4;
                            break;
                        }
                        if (!strcmp(test_string, "AUDITORIUM")) {
                            pInfos[i].uEAXEnv = 6;
                            break;
                        }
                        if (!strcmp(test_string, "CONCERTHALL")) {
                            pInfos[i].uEAXEnv = 7;
                            break;
                        }
                        if (!strcmp(test_string, "HANGAR")) {
                            pInfos[i].uEAXEnv = 10;
                            break;
                        }
                        if (!strcmp(test_string, "CARPETEDHALLWAY")) {
                            pInfos[i].uEAXEnv = 11;
                            break;
                        }
                        if (!strcmp(test_string, "HALLWAY")) {
                            pInfos[i].uEAXEnv = 12;
                            break;
                        }
                        if (!strcmp(test_string, "STONECORRIDOR")) {
                            pInfos[i].uEAXEnv = 13;
                            break;
                        }
                        if (!strcmp(test_string, "ALLEY")) {
                            pInfos[i].uEAXEnv = 14;
                            break;
                        }
                        if (!strcmp(test_string, "QUARRY")) {
                            pInfos[i].uEAXEnv = 18;
                            break;
                        }
                        if (!strcmp(test_string, "PARKINGLOT")) {
                            pInfos[i].uEAXEnv = 20;
                            break;
                        }
                        if (!strcmp(test_string, "SEWERPIPE")) {
                            pInfos[i].uEAXEnv = 21;
                            break;
                        }
                        if (!strcmp(test_string, "DRUGGED")) {
                            pInfos[i].uEAXEnv = 23;
                            break;
                        }
                        if (!strcmp(test_string, "DIZZY")) {
                            pInfos[i].uEAXEnv = 24;
                            break;
                        }
                        if (!strcmp(test_string, "PSYCHOTIC")) {
                            pInfos[i].uEAXEnv = 25;
                            break;
                        }
                        pInfos[i].uEAXEnv = 26;
                    } break;
                }
            } else {
                break_loop = true;
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 31) && !break_loop);
    }

    uNumMaps = 77;
}
// 453F62: using guessed type char Str[32];

//----- (00410D99) --------------------------------------------------------
int MapStats::sub_410D99_get_map_index(int a1) {
    for (int i = 1; i <= pMapStats->uNumMaps; i++) {
        if (_stricmp(pMapStats->pInfos[i].pFilename,
                     pGames_LOD->pSubIndices[a1].pFilename))
            return i;
    }
    Error("Map not found");
    return -1;
}

//----- (004547A3) --------------------------------------------------------
MAP_TYPE MapStats::GetMapInfo(const char *Str2) {
    Assert(uNumMaps >= 2);

    for (uint i = 1; i < uNumMaps; ++i)
        if (!_stricmp(pInfos[i].pFilename, Str2)) return (MAP_TYPE)i;

    Error("Map not found!");
    return (MAP_TYPE)-1;
}

//----- (0044FFD8) --------------------------------------------------------
int MapInfo::SpawnRandomTreasure(SpawnPointMM7 *a2) {
    // MapInfo *v2; // ebx@1
    // SpawnPointMM7 *v3; // esi@1
    // int v4; // eax@1
    int v5;                // edx@1
                           //  int v6; // eax@1
    int v7;                // ecx@1
    int v8;                // ebx@1
    int v9;                // eax@1
    signed int v10;        // ebx@1
    signed int result;     // eax@1
    signed __int64 v12;    // qtt@1
    int v13;               // ebx@1
    signed int v15;        // ebx@20
    unsigned __int16 v16;  // dx@20
                           //  ObjectDesc *v17; // ecx@21
    unsigned __int16 v18;  // ax@24
    int v19;               // ST0C_4@27
    int v20;               // ST08_4@27
    int v21;               // ST04_4@27
    int v22;               // eax@27
    signed int v23;        // ebx@29
    unsigned __int16 v24;  // dx@29
                           //  ObjectDesc *v25; // ecx@30
    unsigned __int16 v26;  // ax@33
    // int v27; // ecx@35
    // int v28; // eax@35
    // int v29; // esi@35
    // __int16 v30; // ax@35
    SpriteObject a1a;  // [sp+Ch] [bp-7Ch]@1
    // int v32; // [sp+7Ch] [bp-Ch]@1
    // int v33; // [sp+80h] [bp-8h]@1
    int v34;  // [sp+84h] [bp-4h]@1

    // auto a1 = this;
    // v2 = a1;
    // v3 = a2;
    // v4 = rand();
    v34 = 0;
    v5 = rand() % 100;
    // v6 = 2 * (v2->Treasure_prob + 7 * v3->uIndex) - 14;
    v7 = (unsigned __int8)byte_4E8168[a2->uIndex - 1][2 * Treasure_prob];
    v8 = (unsigned __int8)byte_4E8168[a2->uIndex - 1][2 * Treasure_prob + 1];
    // v32 = v5;
    // v33 = v7;
    v9 = rand();
    v10 = v8 - v7 + 1;
    v12 = v9;
    result = v9 / v10;
    v13 = v7 + (unsigned __int64)(v12 % v10);
    if (v13 < 7) {
        if (v5 < 20) return result;
        if (v5 >= 60) {
            v19 = a2->vPosition.z;
            v20 = a2->vPosition.y;
            v21 = a2->vPosition.x;
            v22 = rand();
            return sub_450521_ProllyDropItemAt(v13, v22 % 27 + 20, v21, v20,
                                               v19, 0);
        }
        if (a2->uIndex == 1) {
            a1a.containing_item.uItemID = ITEM_GOLD_SMALL;
            v34 = rand() % 51 + 50;
        } else if (a2->uIndex == 2) {
            a1a.containing_item.uItemID = ITEM_GOLD_SMALL;
            v34 = rand() % 101 + 100;
        } else if (a2->uIndex == 3) {
            a1a.containing_item.uItemID = ITEM_GOLD_MEDIUM;
            v34 = rand() % 301 + 200;
        } else if (a2->uIndex == 4) {
            a1a.containing_item.uItemID = ITEM_GOLD_MEDIUM;
            v34 = rand() % 501 + 500;
        } else if (a2->uIndex == 5) {
            a1a.containing_item.uItemID = ITEM_GOLD_LARGE;
            v34 = rand() % 1001 + 1000;
        } else if (a2->uIndex == 6) {
            a1a.containing_item.uItemID = ITEM_GOLD_LARGE;
            v34 = rand() % 3001 + 2000;
        }
        v15 = 0;
        v16 = pItemsTable->pItems[a1a.containing_item.uItemID].uSpriteID;
        a1a.uType =
            (SPRITE_OBJECT_TYPE)pItemsTable->pItems[a1a.containing_item.uItemID]
                .uSpriteID;
        v18 = 0;
        for (int i = 0; i < pObjectList->uNumObjects; i++) {
            if (pObjectList->pObjects[i].uObjectID == v16) {
                v18 = i;
                break;
            }
        }
        a1a.containing_item.SetIdentified();
        a1a.uObjectDescID = v18;
        a1a.containing_item.special_enchantment = (ITEM_ENCHANTMENT)v34;
    } else {
        result = a1a.containing_item.GenerateArtifact();
        if (!result) return result;
        v23 = 0;
        v24 = pItemsTable->pItems[a1a.containing_item.uItemID].uSpriteID;
        a1a.uType =
            (SPRITE_OBJECT_TYPE)pItemsTable->pItems[a1a.containing_item.uItemID]
                .uSpriteID;
        v26 = 0;
        for (int i = 0; i < pObjectList->uNumObjects; i++) {
            if (v24 == pObjectList->pObjects[i].uObjectID) {
                v26 = i;
                break;
            }
        }
        a1a.uObjectDescID = v26;
        a1a.containing_item.Reset();
    }
    a1a.vPosition.y = a2->vPosition.y;
    a1a.uAttributes = 0;
    a1a.uSoundID = 0;
    a1a.uFacing = 0;
    a1a.vPosition.z = a2->vPosition.z;
    a1a.vPosition.x = a2->vPosition.x;
    a1a.spell_skill = 0;
    a1a.spell_level = 0;
    a1a.spell_id = 0;
    a1a.spell_target_pid = 0;
    a1a.spell_caster_pid = 0;
    a1a.uSpriteFrameID = 0;
    a1a.uSectorID =
        pIndoor->GetSector(a2->vPosition.x, a2->vPosition.y, a2->vPosition.z);

    return a1a.Create(0, 0, 0, 0);
}

//----- (004498D5) --------------------------------------------------------
void TeleportToStartingPoint(MapStartPoint point) {
    const char *model_name;  // [sp-4h] [bp-84h]@6
    char pName[128];         // [sp+8h] [bp-78h]@11

    switch (point) {
        case MapStartPoint_Party:
            model_name = "Party Start";
            break;
        case MapStartPoint_North:
            model_name = "North Start";
            break;
        case MapStartPoint_South:
            model_name = "South Start";
            break;
        case MapStartPoint_East:
            model_name = "East Start";
            break;
        case MapStartPoint_West:
            model_name = "West Start";
            break;
        default:
            Error("Invalid enum value: %u", point);
    }

    strcpy(pName, model_name);
    if (pDecorationList->GetDecorIdByName(pName)) {
        if ((signed int)uNumLevelDecorations > 0) {
            for (uint i = 0; i < (signed int)uNumLevelDecorations; ++i) {
                if (pLevelDecorations[i].uDecorationDescID ==
                    (signed __int16)pDecorationList->GetDecorIdByName(pName)) {
                    pParty->vPosition.x = pLevelDecorations[i].vPosition.x;
                    pParty->vPosition.y = pLevelDecorations[i].vPosition.y;
                    pParty->vPosition.z = pLevelDecorations[i].vPosition.z;
                    pParty->uFallStartY = pParty->vPosition.z;
                    pParty->sRotationY =
                        (signed int)(stru_5C6E00->uIntegerHalfPi *
                                     pLevelDecorations[i].field_1A) /
                        90;
                    if (pLevelDecorations[i].field_10_y_rot)
                        pParty->sRotationY =
                            pLevelDecorations[i].field_10_y_rot;
                    pParty->sRotationX = 0;
                    pParty->uFallSpeed = 0;
                }
            }
        }
        if (dword_5B65C0) {
            if (_5B65A8_npcdata_uflags_or_other)
                pParty->vPosition.x = _5B65A8_npcdata_uflags_or_other;
            if (_5B65AC_npcdata_fame_or_other)
                pParty->vPosition.y = _5B65AC_npcdata_fame_or_other;
            if (_5B65B0_npcdata_rep_or_other) {
                pParty->vPosition.z = _5B65B0_npcdata_rep_or_other;
                pParty->uFallStartY = _5B65B0_npcdata_rep_or_other;
            }
            if (_5B65B4_npcdata_loword_house_or_other != -1)
                pParty->sRotationY = _5B65B4_npcdata_loword_house_or_other;
            if (_5B65B8_npcdata_hiword_house_or_other)
                pParty->sRotationX = _5B65B8_npcdata_hiword_house_or_other;
            if (dword_5B65BC) pParty->uFallSpeed = dword_5B65BC;
        }
        _5B65B4_npcdata_loword_house_or_other = -1;
        dword_5B65C0 = 0;
        dword_5B65BC = 0;
        _5B65B8_npcdata_hiword_house_or_other = 0;
        _5B65B0_npcdata_rep_or_other = 0;
        _5B65AC_npcdata_fame_or_other = 0;
        _5B65A8_npcdata_uflags_or_other = 0;
    }
}
