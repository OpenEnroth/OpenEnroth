#include "Engine/MapInfo.h"

#include <cstdlib>
#include <sstream>

#include "Engine/Engine.h"

#include "Engine/Graphics/Indoor.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "LOD.h"

#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "OurMath.h"
#include "Party.h"

const char *location_type[] = {
    "GENERIC",
    "PADDEDCELL",
    "ROOM",
    "BATHROOM",
    "LIVINGROOM",
    "STONEROOM",
    "AUDITORIUM",
    "CONCERTHALL",
    "CAVE",
    "ARENA",
    "HANGAR",
    "CARPETEDHALLWAY",
    "HALLWAY",
    "STONECORRIDOR",
    "ALLEY",
    "FOREST",
    "CITY",
    "MOUNTAIN",
    "QUARRY",
    "PLAINS",
    "PARKINGLOT",
    "SEWERPIPE",
    "UNDERWATER",
    "DRUGGED",
    "DIZZY",
    "PSYCHOTIC"
};

void MapStats::Initialize() {
    String pMapStatsTXT = (char*)pEvents_LOD->LoadCompressedTexture("MapStats.txt");
    std::stringstream stream(pMapStatsTXT);
    String tmpString;
    std::getline(stream, tmpString);
    std::getline(stream, tmpString);
    std::getline(stream, tmpString);

    char work_str[32];
    int work_str_pos;
    int work_str_len;

    size_t i = 1;
    while (!stream.eof()) {
        std::getline(stream, tmpString);
        std::stringstream line(tmpString);
        size_t decode_step = 0;
        while (!line.eof()) {
            std::getline(line, tmpString, '\t');
            char test_string[1024];
            strcpy(test_string, tmpString.c_str());
            switch (decode_step) {
                case 1:
                    pInfos[i].pName = RemoveQuotes(test_string);  // randoms crashes here
                    break;
                case 2:
                    pInfos[i].pFilename = MakeLower(RemoveQuotes(test_string));
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
                    pInfos[i].pEncounterMonster1Texture = RemoveQuotes(test_string);
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
                        pInfos[i].uEncounterMonster1AtLeast = atoi(work_str);
                        if (work_str_pos < work_str_len)
                            pInfos[i].uEncounterMonster1AtMost = atoi(&work_str[work_str_pos + 1]);
                        else
                            pInfos[i].uEncounterMonster1AtMost = pInfos[i].uEncounterMonster1AtLeast;
                    }
                    break;
                case 20:
                    pInfos[i].pEncounterMonster2Texture = RemoveQuotes(test_string);
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
                        pInfos[i].uEncounterMonster2AtLeast = atoi(work_str);
                        if (work_str_pos < work_str_len)
                            pInfos[i].uEncounterMonster2AtMost = atoi(&work_str[work_str_pos + 1]);
                        else
                            pInfos[i].uEncounterMonster2AtMost = pInfos[i].uEncounterMonster2AtLeast;
                    }
                    break;
                case 24:
                    pInfos[i].pEncounterMonster3Texture = RemoveQuotes(test_string);
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
                        pInfos[i].uEncounterMonster3AtLeast = atoi(work_str);
                        if (work_str_pos < work_str_len)
                            pInfos[i].uEncounterMonster3AtMost = atoi(&work_str[work_str_pos + 1]);
                        else
                            pInfos[i].uEncounterMonster3AtMost = pInfos[i].uEncounterMonster3AtLeast;
                    }
                    break;
                case 28:
                    pInfos[i].uRedbookTrackID = atoi(test_string);
                    break;
                case 29: {
                    pInfos[i].uEAXEnv = 0xff;
                    for (int j = 0; j < 25; j++) {
                        if (!strcmp(test_string, location_type[j])) {
                            pInfos[i].uEAXEnv = j;
                            break;
                        }
                    }
                    if (pInfos[i].uEAXEnv == 0xff) {
                        pInfos[i].uEAXEnv = 26;
                    }
                } break;
            }
            decode_step++;
        }
        i++;
    }

    uNumMaps = i - 1;
}

int MapStats::sub_410D99_get_map_index(int a1) {
    String name = pGames_LOD->GetSubNodeName(a1);

    for (int i = 1; i <= pMapStats->uNumMaps; i++) {
        if (pMapStats->pInfos[i].pFilename == name) {
            return i;
        }
    }
    Error("Map not found");
    return -1;
}

MAP_TYPE MapStats::GetMapInfo(const String &Str2) {
    Assert(uNumMaps >= 2);

    String map_name = MakeLower(Str2);

    for (uint i = 1; i < uNumMaps; ++i) {
        if (pInfos[i].pFilename == map_name) {
            return (MAP_TYPE)i;
        }
    }

    Error("Map not found!");
    return (MAP_TYPE)-1;
}

int MapInfo::SpawnRandomTreasure(SpawnPointMM7 *a2) {
    SpriteObject a1a;
    a1a.containing_item.Reset();

    int v34 = 0;
    int v5 = rand() % 100;
    int v7 = (uint8_t)byte_4E8168[a2->uIndex - 1][2 * Treasure_prob];
    int v8 = (uint8_t)byte_4E8168[a2->uIndex - 1][2 * Treasure_prob + 1];
    int v9 = rand();
    int v10 = v8 - v7 + 1;
    int64_t v12 = v9;
    int result = v9 / v10;
    int v13 = v7 + (uint64_t)(v12 % v10);
    if (v13 < 7) {
        if (v5 < 20) return result;
        if (v5 >= 60) {
            return sub_450521_ProllyDropItemAt(v13, rand() % 27 + 20,
                                               a2->vPosition.x,
                                               a2->vPosition.y,
                                               a2->vPosition.z, 0);
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
        a1a.uType = (SPRITE_OBJECT_TYPE)pItemsTable->pItems[a1a.containing_item.uItemID].uSpriteID;
        a1a.containing_item.SetIdentified();
        a1a.uObjectDescID = pObjectList->ObjectIDByItemID(a1a.uType);
        a1a.containing_item.special_enchantment = (ITEM_ENCHANTMENT)v34;
    } else {
        result = a1a.containing_item.GenerateArtifact();
        if (!result) return result;
        a1a.uType = (SPRITE_OBJECT_TYPE)pItemsTable->pItems[a1a.containing_item.uItemID].uSpriteID;
        a1a.uObjectDescID = pObjectList->ObjectIDByItemID(a1a.uType);
        a1a.containing_item.Reset();  // ?? this needs checking
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
    a1a.uSectorID = pIndoor->GetSector(a2->vPosition.x, a2->vPosition.y, a2->vPosition.z);

    return a1a.Create(0, 0, 0, 0);
}

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
        if (uNumLevelDecorations > 0) {
            for (size_t i = 0; i < uNumLevelDecorations; ++i) {
                if (pLevelDecorations[i].uDecorationDescID == pDecorationList->GetDecorIdByName(pName)) {
                    pParty->vPosition.x = pLevelDecorations[i].vPosition.x;
                    pParty->vPosition.y = pLevelDecorations[i].vPosition.y;
                    pParty->vPosition.z = pLevelDecorations[i].vPosition.z;
                    pParty->uFallStartY = pParty->vPosition.z;
                    pParty->sRotationY = (int)(stru_5C6E00->uIntegerHalfPi *
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
