#include "MapInfo.h"

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>

#include "Utility/Memory/Blob.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Transformations.h"

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
                    pInfos[i].name = removeQuotes(test_string);  // randoms crashes here  // got 1 too
                    break;
                case 2:
                    pInfos[i].fileName = ascii::toLower(removeQuotes(test_string));
                    break;
                case 3:
                    pInfos[i].numResets = atoi(test_string);
                    break;
                case 4:
                    pInfos[i].firstVisitedAt = atoi(test_string);
                    break;
                case 5:
                    pInfos[i].perceptionDifficulty = atoi(test_string);
                    break;
                case 6:
                    pInfos[i].respawnIntervalDays = atoi(test_string);
                    break;
                case 7:
                    pInfos[i].alertDays = atoi(test_string);
                    break;
                case 8:
                    pInfos[i].baseStealingFine = atoi(test_string);
                    break;
                case 9:
                    pInfos[i].disarmDifficulty = atoi(test_string);
                    break;
                case 10:
                    pInfos[i].trapDamageD20DiceCount = atoi(test_string);
                    break;
                case 11:
                    pInfos[i].mapTreasureLevel = static_cast<MapTreasureLevel>(atoi(test_string));  // treasure levels 0-6
                    break;
                case 12:
                    pInfos[i].encounterChance = atoi(test_string);
                    break;
                case 13:
                    pInfos[i].encounter1Chance = atoi(test_string);
                    break;
                case 14:
                    pInfos[i].encounter2Chance = atoi(test_string);
                    break;
                case 15:
                    pInfos[i].encounter3Chance = atoi(test_string);
                    break;
                case 16:
                    pInfos[i].encounter1MonsterTexture = removeQuotes(test_string);
                    break;
                case 18:
                    pInfos[i].Dif_M1 = atoi(test_string);
                    break;
                case 19:
                    pInfos[i].encounter1MinCount = 1;
                    pInfos[i].encounter1MaxCount = 1;
                    strncpy(work_str, test_string, sizeof(work_str) - 1);
                    work_str_pos = 0;
                    work_str_len = strlen(work_str);
                    if (work_str_len) {
                        while (work_str[work_str_pos] != '-') {
                            ++work_str_pos;
                            if (work_str_pos >= work_str_len) break;
                        }
                        work_str[work_str_pos] = 0;
                        pInfos[i].encounter1MinCount = atoi(work_str);
                        if (work_str_pos < work_str_len)
                            pInfos[i].encounter1MaxCount = atoi(&work_str[work_str_pos + 1]);
                        else
                            pInfos[i].encounter1MaxCount = pInfos[i].encounter1MinCount;
                    }
                    break;
                case 20:
                    pInfos[i].encounter2MonsterTexture = removeQuotes(test_string);
                    break;
                case 22:
                    pInfos[i].Dif_M2 = atoi(test_string);
                    break;
                case 23:
                    pInfos[i].encounter2MinCount = 1;
                    pInfos[i].encounter2MaxCount = 1;
                    strncpy(work_str, test_string, sizeof(work_str) - 1);
                    work_str_pos = 0;
                    work_str_len = strlen(work_str);
                    if (work_str_len) {
                        while (work_str[work_str_pos] != '-') {
                            ++work_str_pos;
                            if (work_str_pos >= work_str_len) break;
                        }
                        work_str[work_str_pos] = 0;
                        pInfos[i].encounter2MinCount = atoi(work_str);
                        if (work_str_pos < work_str_len)
                            pInfos[i].encounter2MaxCount = atoi(&work_str[work_str_pos + 1]);
                        else
                            pInfos[i].encounter2MaxCount = pInfos[i].encounter2MinCount;
                    }
                    break;
                case 24:
                    pInfos[i].encounter3MonsterTexture = removeQuotes(test_string);
                    break;
                case 26:
                    pInfos[i].Dif_M3 = atoi(test_string);
                    break;
                case 27:
                    pInfos[i].encounter3MinCount = 1;
                    pInfos[i].encounter3MaxCount = 1;
                    strncpy(work_str, test_string, sizeof(work_str) - 1);
                    work_str_pos = 0;
                    work_str_len = strlen(work_str);
                    if (work_str_len) {
                        while (work_str[work_str_pos] != '-') {
                            ++work_str_pos;
                            if (work_str_pos >= work_str_len) break;
                        }
                        work_str[work_str_pos] = 0;
                        pInfos[i].encounter3MinCount = atoi(work_str);
                        if (work_str_pos < work_str_len)
                            pInfos[i].encounter3MaxCount = atoi(&work_str[work_str_pos + 1]);
                        else
                            pInfos[i].encounter3MaxCount = pInfos[i].encounter3MinCount;
                    }
                    break;
                case 28:
                    pInfos[i].musicId = static_cast<MusicId>(atoi(test_string));
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

MapId MapStats::GetMapInfo(std::string_view Str2) {
    std::string map_name = ascii::toLower(Str2);

    for (MapId i : pInfos.indices()) {
        if (pInfos[i].fileName == map_name) {
            return i;
        }
    }

    assert(false);
    return MAP_INVALID;
}
