#include "MapInfo.h"

#include <array>
#include <map>
#include <string>

#include "Library/Tsv/TsvReader.h"
#include "Library/Serialization/Serialization.h"

#include "Utility/MapAccess.h"
#include "Utility/Memory/Blob.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Transformations.h"

MapStats *pMapStats;

void MapStats::Initialize(const Blob &mapStats) {
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

    for (TsvLine line : TsvReader(mapStats).drop(3).skip(&TsvLine::isBlank)) {
        MapId mapId = static_cast<MapId>(line[0].as<int>());
        MapInfo &info = pInfos[mapId];
        info.name = line[1];
        info.fileName = ascii::toLower(line[2]);
        info.numResets = line[3].as<int>();
        info.firstVisitedAt = line[4].as<int>();
        info.perceptionDifficulty = line[5].as<int>();
        info.respawnIntervalDays = line[6].as<int>();
        info.alertDays = line[7].as<int>();
        info.baseStealingFine = line[8].as<int>();
        info.disarmDifficulty = line[9].as<int>();
        info.trapDamageD20DiceCount = line[10].as<int>();
        info.mapTreasureLevel = static_cast<MapTreasureLevel>(line[11].as<int>());
        info.encounterChance = line[12].as<int>();
        info.encounter1Chance = line[13].as<int>();
        info.encounter2Chance = line[14].as<int>();
        info.encounter3Chance = line[15].as<int>();
        info.encounter1MonsterInternalName = line[16];
        info.Dif_M1 = line[18].as<int>();
        parseRange(line[19], &info.encounter1MinCount, &info.encounter1MaxCount);
        info.encounter2MonsterInternalName = line[20];
        info.Dif_M2 = line[22].as<int>();
        parseRange(line[23], &info.encounter2MinCount, &info.encounter2MaxCount);
        info.encounter3MonsterInternalName = line[24];
        info.Dif_M3 = line[26].as<int>();
        parseRange(line[27], &info.encounter3MinCount, &info.encounter3MaxCount);
        info.musicId = static_cast<MusicId>(line[28].as<int>());
        info.uEAXEnv = valueOr(eaxEnvMap, std::string(line[29]), 26);
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
