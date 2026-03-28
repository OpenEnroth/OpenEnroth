#include "HouseTable.h"

#include <cstring>
#include <map>
#include <string>

#include "Engine/Data/HouseEnumFunctions.h"

#include "Utility/MapAccess.h"
#include "Utility/Memory/Blob.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Transformations.h"

IndexedArray<HouseData, HOUSE_FIRST, HOUSE_LAST> houseTable;

void initializeHouses(const Blob &houses) {
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

    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    std::string txtRaw(houses.str());
    strtok(txtRaw.data(), "\r");
    strtok(NULL, "\r");

    for (HouseId houseId : allHouses()) {
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
                case 2:
                    houseTable[houseId].uType = valueOr(houseTypeMap, test_string, HOUSE_TYPE_MERCENARY_GUILD);
                    break;

                case 4:
                    houseTable[houseId].uAnimationID = atoi(test_string);
                    break;
                case 5:
                    houseTable[houseId].name = removeQuotes(test_string);
                    break;
                case 6:
                    houseTable[houseId].pProprieterName = removeQuotes(test_string);
                    break;
                case 7:
                    houseTable[houseId].pProprieterTitle = removeQuotes(test_string);
                    break;
                case 8:
                    houseTable[houseId].field_14 = atoi(test_string);
                    break;
                case 9:
                    houseTable[houseId]._state = atoi(test_string);
                    break;
                case 10:
                    houseTable[houseId]._rep = atoi(test_string);
                    break;
                case 11:
                    houseTable[houseId]._per = atoi(test_string);
                    break;
                case 12:
                    houseTable[houseId].fPriceMultiplier = atof(test_string);
                    break;
                case 13:
                    houseTable[houseId].flt_24 = atof(test_string);
                    break;
                case 15:
                    houseTable[houseId].generation_interval_days = atoi(test_string);
                    break;
                case 18:
                    houseTable[houseId].uOpenTime = atoi(test_string);
                    break;
                case 19:
                    houseTable[houseId].uCloseTime = atoi(test_string);
                    break;
                case 20:
                    houseTable[houseId].uExitPicID = atoi(test_string);
                    break;
                case 21:
                    houseTable[houseId].uExitMapID = static_cast<MapId>(atoi(test_string));
                    break;
                case 22:
                    houseTable[houseId]._quest_bit = static_cast<QuestBit>(atoi(test_string));
                    break;
                case 23:
                    houseTable[houseId].pEnterText = removeQuotes(test_string);
                    break;
                }
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 24) && !break_loop);
    }
}
