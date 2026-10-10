#include "HouseTable.h"

#include <array>
#include <map>
#include <string>

#include "Engine/Data/HouseEnums.h"

#include "Library/Serialization/Serialization.h"

#include "Utility/MapAccess.h"
#include "Utility/Memory/Blob.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

IndexedArray<HouseData, HOUSE_FIRST, HOUSE_LAST> houseTable;

static int intOrZero(std::string_view token) {
    return token.empty() ? 0 : fromString<int>(token);
}

static float floatOrZero(std::string_view token) {
    return token.empty() ? 0.0f : fromString<float>(token);
}

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

    for (std::string_view line : split(houses.str()).by("\r\n").drop(2).skip("")) {
        std::array<std::string_view, 24> tokens = split(line).by('\t').resize(24, ""); // Some lines have only ~12 cols.

        // TODO(captainurist): We don't check if int is in range. A better way would be to deal away with enums
        //                     entirely, and just use typed ids. Do this once we iron out the details of how #mm6
        //                     enums will be handled by the engine. Also apply to other table parsers.
        HouseId houseId = static_cast<HouseId>(fromString<int>(tokens[0]));
        houseTable[houseId].uType = tokens[2].empty() ? HOUSE_TYPE_INVALID : valueOr(houseTypeMap, tokens[2], HOUSE_TYPE_MERCENARY_GUILD);
        houseTable[houseId].uAnimationID = intOrZero(tokens[4]);
        houseTable[houseId].name = unquote(tokens[5]);
        houseTable[houseId].pProprieterName = unquote(tokens[6]);
        houseTable[houseId].pProprieterTitle = unquote(tokens[7]);
        houseTable[houseId].fPriceMultiplier = floatOrZero(tokens[12]);
        houseTable[houseId].flt_24 = floatOrZero(tokens[13]);
        houseTable[houseId].generation_interval_days = intOrZero(tokens[15]);
        houseTable[houseId].uOpenTime = intOrZero(tokens[18]);
        houseTable[houseId].uCloseTime = intOrZero(tokens[19]);
        houseTable[houseId].uExitPicID = intOrZero(tokens[20]);
        houseTable[houseId].uExitMapID = static_cast<MapId>(intOrZero(tokens[21]));
        houseTable[houseId]._quest_bit = static_cast<QuestBit>(intOrZero(tokens[22]));
    }
}
