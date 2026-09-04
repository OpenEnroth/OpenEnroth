#include "MapInfo.h"

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

    for (TsvLine cells : TsvReader(mapStats).drop(3).skip(&TsvLine::isBlank)) {
        MapId mapId = static_cast<MapId>(cells[0].as<int>());
        MapInfo &info = pInfos[mapId];
        info.name = cells[1];
        info.fileName = ascii::toLower(cells[2]);
        info.numResets = cells[3].as<int>();
        info.firstVisitedAt = cells[4].as<int>();
        info.perceptionDifficulty = cells[5].as<int>();
        info.respawnIntervalDays = cells[6].as<int>();
        info.alertDays = cells[7].as<int>();
        info.baseStealingFine = cells[8].as<int>();
        info.disarmDifficulty = cells[9].as<int>();
        info.trapDamageD20DiceCount = cells[10].as<int>();
        info.mapTreasureLevel = static_cast<MapTreasureLevel>(cells[11].as<int>());
        info.encounterChance = cells[12].as<int>();
        info.encounter1Chance = cells[13].as<int>();
        info.encounter2Chance = cells[14].as<int>();
        info.encounter3Chance = cells[15].as<int>();
        info.encounter1MonsterInternalName = cells[16];
        info.Dif_M1 = cells[18].as<int>();
        parseRange(cells[19], &info.encounter1MinCount, &info.encounter1MaxCount);
        info.encounter2MonsterInternalName = cells[20];
        info.Dif_M2 = cells[22].as<int>();
        parseRange(cells[23], &info.encounter2MinCount, &info.encounter2MaxCount);
        info.encounter3MonsterInternalName = cells[24];
        info.Dif_M3 = cells[26].as<int>();
        parseRange(cells[27], &info.encounter3MinCount, &info.encounter3MaxCount);
        info.musicId = static_cast<MusicId>(cells[28].as<int>());
        info.uEAXEnv = valueOr(eaxEnvMap, std::string(cells[29]), 26);
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
