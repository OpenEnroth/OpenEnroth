#include "HouseTable.h"

#include <map>
#include <string>

#include "Engine/Data/HouseEnums.h"

#include "Library/Tsv/TsvReader.h"

#include "Utility/MapAccess.h"
#include "Utility/Memory/Blob.h"
#include "Utility/String/Ascii.h"

IndexedArray<HouseData, HOUSE_FIRST, HOUSE_LAST> houseTable;

void initializeHouses(const Blob &houses) {
    // 2devents.txt table structure (column names are the headers from the data file):
    //  0: "#"                  - house id
    //  1: "#"                  - per-type sequence number, resets at each new Type         (not used)
    //  2: "Type"               - house type                                                (not localized)
    //  3: "Map"                - map id this building lives on                             (not used)
    //  4: "Picture"            - index into `pAnimatedRooms`, for npc id, video & sound
    //  5: "Name"               - house name                                                (localized)
    //  6: "Proprietor Name"                                                                (localized)
    //  7: "Proprietor Title"                                                               (localized)
    //  8: "Picture"            - always 0                                                  (not used in MM7)
    //  9: "State"              - always 0                                                  (not used in MM7)
    // 10: "Rep"                - always 0, reputation?                                     (not used in MM7)
    // 11: "Per"                - always 0                                                  (not used in MM7)
    // 12: "Val"                - shop price multiplier, float
    // 13: "A"                  - skill/spell price multiplier, float
    // 14: "B"                  - always empty
    // 15: "C"                  - item-generation interval, days
    // 16: "Notes:"             - mostly empty, an alternative index into `pAnimatedRooms`,
    //                            points at the base entry of a race-tier triplet           (not used)
    // 17: "Notes(2):"          - max trainable level for Training houses                   (not used)
    // 18: "Open"               - opening hour, 0-24
    // 19: "Closed"             - closing hour, 0-24
    // 20: "Pic"                - exit picture id                                           (not used in MM7)
    // 21: "Map"                - exit map id                                               (not used in MM7)
    // 22: "Restrictions"       - exit gating quest bit                                     (not used in MM7)
    // 23: "Text"               - exit text                                                 (not used in MM7)
    static const std::map<std::string, HouseType, ascii::NoCaseLess> houseTypeMap = {
        {"Weapon Shop", HOUSE_TYPE_WEAPON_SHOP},
        {"Armor Shop", HOUSE_TYPE_ARMOR_SHOP},
        {"Magic Shop", HOUSE_TYPE_MAGIC_SHOP},
        {"Alchemist", HOUSE_TYPE_ALCHEMY_SHOP},
        {"Stables", HOUSE_TYPE_STABLE},
        {"Boats", HOUSE_TYPE_BOAT},
        {"Temple", HOUSE_TYPE_TEMPLE},
        {"Training", HOUSE_TYPE_TRAINING_GROUND},
        {"Town Hall", HOUSE_TYPE_TOWN_HALL},
        {"Tavern", HOUSE_TYPE_TAVERN},
        {"Bank", HOUSE_TYPE_BANK},
        {"Fire Guild", HOUSE_TYPE_FIRE_GUILD},
        {"Air Guild", HOUSE_TYPE_AIR_GUILD},
        {"Water Guild", HOUSE_TYPE_WATER_GUILD},
        {"Earth Guild", HOUSE_TYPE_EARTH_GUILD},
        {"Spirit Guild", HOUSE_TYPE_SPIRIT_GUILD},
        {"Mind Guild", HOUSE_TYPE_MIND_GUILD},
        {"Body Guild", HOUSE_TYPE_BODY_GUILD},
        {"Light Guild", HOUSE_TYPE_LIGHT_GUILD},
        {"Dark Guild", HOUSE_TYPE_DARK_GUILD},
        {"Element Guild", HOUSE_TYPE_ELEMENTAL_GUILD}, // This is MM6 only.
        {"Self Guild", HOUSE_TYPE_SELF_GUILD},
        {"Mirrored Path Guild", HOUSE_TYPE_MIRRORED_PATH_GUILD},
        {"Mercenary Guild", HOUSE_TYPE_TOWN_HALL}, // This is MM6 only. TODO(captainurist): Is this right and not Merc Guild (18)?
    };

    for (TsvLine cells : TsvReader(houses).drop(2).skip(&TsvLine::isBlank)) {
        // Lines are ragged and many numeric cells are empty, so those default to 0.

        // TODO(captainurist): We don't check if int is in range. A better way would be to deal away with enums
        //                     entirely, and just use typed ids. Do this once we iron out the details of how #mm6
        //                     enums will be handled by the engine. Also apply to other table parsers.
        HouseId houseId = static_cast<HouseId>(cells[0].as<int>());
        houseTable[houseId].uType = valueOr(houseTypeMap, cells[2], HOUSE_TYPE_MERCENARY_GUILD);
        houseTable[houseId].uAnimationID = cells[4].empty() ? 0 : cells[4].as<int>();
        houseTable[houseId].name = cells[5];
        houseTable[houseId].pProprieterName = cells[6];
        houseTable[houseId].pProprieterTitle = cells[7];
        houseTable[houseId].fPriceMultiplier = cells[12].empty() ? 0 : cells[12].as<float>();
        houseTable[houseId].flt_24 = cells[13].empty() ? 0 : cells[13].as<float>();
        houseTable[houseId].generation_interval_days = cells[15].empty() ? 0 : cells[15].as<int>();
        houseTable[houseId].uOpenTime = cells[18].empty() ? 0 : cells[18].as<int>();
        houseTable[houseId].uCloseTime = cells[19].empty() ? 0 : cells[19].as<int>();
        houseTable[houseId].uExitPicID = cells[20].empty() ? 0 : cells[20].as<int>();
        houseTable[houseId].uExitMapID = static_cast<MapId>(cells[21].empty() ? 0 : cells[21].as<int>());
        houseTable[houseId]._quest_bit = static_cast<QuestBit>(cells[22].empty() ? 0 : cells[22].as<int>());
    }
}
