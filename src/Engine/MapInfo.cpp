#include "MapInfo.h"

#include <array>
#include <map>
#include <string>

#include "Library/Serialization/Serialization.h"

#include "Utility/MapAccess.h"
#include "Utility/Memory/Blob.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

MapStats *pMapStats;

void MapStats::Initialize(std::string_view mapStats) {
    // mapstats.txt table structure: map id | name (localized) | file name | ... |
    //                               map designer (set only in mm6, not used) | dev notes | parent map (not used).
    static const std::map<std::string, uint8_t, ascii::NoCaseLess> eaxEnvMap = {
        {"GENERIC", 0},
        {"PADDEDCELL", 1},
        {"ROOM", 2},
        {"BATHROOM", 3},
        {"LIVINGROOM", 4},
        {"STONEROOM", 5},
        {"AUDITORIUM", 6},
        {"CONCERTHALL", 7},
        {"CAVE", 8},
        {"ARENA", 9},
        {"HANGAR", 10},
        {"CARPETEDHALLWAY", 11},
        {"HALLWAY", 12},
        {"STONECORRIDOR", 13},
        {"ALLEY", 14},
        {"FOREST", 15},
        {"CITY", 16},
        {"MOUNTAIN", 17},
        {"QUARRY", 18},
        {"PLAINS", 19},
        {"PARKINGLOT", 20},
        {"SEWERPIPE", 21},
        {"UNDERWATER", 22},
        {"DRUGGED", 23},
        {"DIZZY", 24},
        {"PSYCHOTIC", 25},
    };

    auto parseRange = [](std::string_view s, uint8_t *minOut, uint8_t *maxOut) {
        // Range cells can have leading whitespace (e.g. " 2-5"), so trim before each fromString.
        auto dash = s.find('-');
        if (dash == std::string_view::npos) {
            *minOut = fromString<int>(trim(s));
            *maxOut = *minOut;
        } else {
            *minOut = fromString<int>(trim(s.substr(0, dash)));
            *maxOut = fromString<int>(trim(s.substr(dash + 1)));
        }
    };

    for (std::string_view line : split(mapStats).by("\r\n").drop(3).skip("")) {
        std::array<std::string_view, 30> tokens = split(line).by('\t');
        MapId mapId = static_cast<MapId>(fromString<int>(tokens[0]));
        MapInfo &info = pInfos[mapId];
        info.name = unquote(tokens[1]);
        info.fileName = ascii::toLower(unquote(tokens[2]));
        info.numResets = fromString<int>(tokens[3]);
        info.firstVisitedAt = fromString<int>(tokens[4]);
        info.perceptionDifficulty = fromString<int>(tokens[5]);
        info.respawnIntervalDays = fromString<int>(tokens[6]);
        info.alertDays = fromString<int>(tokens[7]);
        info.baseStealingFine = fromString<int>(tokens[8]);
        info.disarmDifficulty = fromString<int>(tokens[9]);
        info.trapDamageD20DiceCount = fromString<int>(tokens[10]);
        info.mapTreasureLevel = static_cast<MapTreasureLevel>(fromString<int>(tokens[11]));
        info.encounterChance = fromString<int>(tokens[12]);
        info.encounter1Chance = fromString<int>(tokens[13]);
        info.encounter2Chance = fromString<int>(tokens[14]);
        info.encounter3Chance = fromString<int>(tokens[15]);
        info.encounter1MonsterInternalName = unquote(tokens[16]);
        info.Dif_M1 = fromString<int>(tokens[18]);
        parseRange(tokens[19], &info.encounter1MinCount, &info.encounter1MaxCount);
        info.encounter2MonsterInternalName = unquote(tokens[20]);
        info.Dif_M2 = fromString<int>(tokens[22]);
        parseRange(tokens[23], &info.encounter2MinCount, &info.encounter2MaxCount);
        info.encounter3MonsterInternalName = unquote(tokens[24]);
        info.Dif_M3 = fromString<int>(tokens[26]);
        parseRange(tokens[27], &info.encounter3MinCount, &info.encounter3MaxCount);
        info.musicId = static_cast<MusicId>(fromString<int>(tokens[28]));
        info.uEAXEnv = valueOr(eaxEnvMap, std::string(tokens[29]), 26);
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
