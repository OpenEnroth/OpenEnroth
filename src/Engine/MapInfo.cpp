#include "MapInfo.h"

#include <cstdlib>
#include <sstream>

#include "Utility/Memory/Blob.h"
#include "Utility/Workaround/ToUnderlying.h"
#include "Utility/String.h"

MapStats *pMapStats;

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

void MapStats::Initialize(const Blob &mapStats) {
    std::string pMapStatsTXT(mapStats.string_view());
    std::stringstream stream(pMapStatsTXT);
    std::string tmpString;
    std::getline(stream, tmpString);
    std::getline(stream, tmpString);
    std::getline(stream, tmpString);

    char work_str[32];
    int work_str_pos;
    int work_str_len;

    MapId i = MAP_FIRST;
    while (!stream.eof()) {
        std::getline(stream, tmpString);
        std::stringstream line(tmpString);
        size_t decode_step = 0;
        while (!line.eof()) {
            std::getline(line, tmpString, '\t');
            char test_string[1024];
            strncpy(test_string, tmpString.c_str(), sizeof(test_string) - 1);
            switch (decode_step) {
                case 1:
                    pInfos[i].pName = removeQuotes(test_string);  // randoms crashes here  // got 1 too
                    break;
                case 2:
                    pInfos[i].pFilename = toLower(removeQuotes(test_string));
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
                    pInfos[i].Treasure_prob = MAP_TREASURE_LEVEL(atoi(test_string));  // treasure levels 0-6
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
                    pInfos[i].pEncounterMonster1Texture = removeQuotes(test_string);
                    break;
                case 18:
                    pInfos[i].Dif_M1 = atoi(test_string);
                    break;
                case 19:
                    pInfos[i].uEncounterMonster1AtLeast = 1;
                    pInfos[i].uEncounterMonster1AtMost = 1;
                    strncpy(work_str, test_string, sizeof(work_str) - 1);
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
                    pInfos[i].pEncounterMonster2Texture = removeQuotes(test_string);
                    break;
                case 22:
                    pInfos[i].Dif_M2 = atoi(test_string);
                    break;
                case 23:
                    pInfos[i].uEncounterMonster2AtLeast = 1;
                    pInfos[i].uEncounterMonster2AtMost = 1;
                    strncpy(work_str, test_string, sizeof(work_str) - 1);
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
                    pInfos[i].pEncounterMonster3Texture = removeQuotes(test_string);
                    break;
                case 26:
                    pInfos[i].Dif_M3 = atoi(test_string);
                    break;
                case 27:
                    pInfos[i].uEncounterMonster3AtLeast = 1;
                    pInfos[i].uEncounterMonster3AtMost = 1;
                    strncpy(work_str, test_string, sizeof(work_str) - 1);
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
                    pInfos[i].uRedbookTrackID = static_cast<MusicId>(atoi(test_string));
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
        i = static_cast<MapId>(std::to_underlying(i) + 1);
    }
}

MapId MapStats::GetMapInfo(const std::string &Str2) {
    std::string map_name = toLower(Str2);

    for (MapId i : pInfos.indices()) {
        if (pInfos[i].pFilename == map_name) {
            return i;
        }
    }

    assert(false);
    return MAP_INVALID;
}
