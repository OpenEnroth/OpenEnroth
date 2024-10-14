#include "BuildingTable.h"

#include <cstring>
#include <string>

#include "Utility/Memory/Blob.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Transformations.h"

IndexedArray<HouseData, HOUSE_FIRST, HOUSE_LAST> buildingTable;

void initializeBuildings(const Blob &buildings) {
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    std::string txtRaw(buildings.string_view());
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
                {
                    if (ascii::noCaseStartsWith(test_string, "wea")) {
                        buildingTable[houseId].uType = HOUSE_TYPE_WEAPON_SHOP;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "arm")) {
                        buildingTable[houseId].uType = HOUSE_TYPE_ARMOR_SHOP;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "mag")) {
                        buildingTable[houseId].uType = HOUSE_TYPE_MAGIC_SHOP;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "alc")) {
                        buildingTable[houseId].uType = HOUSE_TYPE_ALCHEMY_SHOP;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "sta")) {
                        buildingTable[houseId].uType = HOUSE_TYPE_STABLE;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "boa")) {
                        buildingTable[houseId].uType = HOUSE_TYPE_BOAT;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "tem")) {
                        buildingTable[houseId].uType = HOUSE_TYPE_TEMPLE;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "tra")) {
                        buildingTable[houseId].uType = HOUSE_TYPE_TRAINING_GROUND;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "tow")) {
                        buildingTable[houseId].uType = HOUSE_TYPE_TOWN_HALL;
                        break;
                    }

                    if (ascii::noCaseStartsWith(test_string, "tav")) {
                        buildingTable[houseId].uType = HOUSE_TYPE_TAVERN;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "ban")) {
                        buildingTable[houseId].uType = HOUSE_TYPE_BANK;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "fir")) {
                        buildingTable[houseId].uType = HOUSE_TYPE_FIRE_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "air")) {
                        buildingTable[houseId].uType = HOUSE_TYPE_AIR_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "wat")) {
                        buildingTable[houseId].uType = HOUSE_TYPE_WATER_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "ear")) {
                        buildingTable[houseId].uType = HOUSE_TYPE_EARTH_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "spi")) {
                        buildingTable[houseId].uType = HOUSE_TYPE_SPIRIT_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "min")) {
                        buildingTable[houseId].uType = HOUSE_TYPE_MIND_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "bod")) {
                        buildingTable[houseId].uType = HOUSE_TYPE_BODY_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "lig")) {
                        buildingTable[houseId].uType = HOUSE_TYPE_LIGHT_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "dar")) {
                        buildingTable[houseId].uType = HOUSE_TYPE_DARK_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "ele")) { // "Element Guild" from mm6
                        buildingTable[houseId].uType = HOUSE_TYPE_ELEMENTAL_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "sel")) {
                        buildingTable[houseId].uType = HOUSE_TYPE_SELF_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "mir")) {
                        buildingTable[houseId].uType = HOUSE_TYPE_MIRRORED_PATH_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "mer")) { // "Thieves Guild" from mm6
                        buildingTable[houseId].uType = HOUSE_TYPE_TOWN_HALL; //TODO: Is this right and not Merc Guild (18)?
                        break;
                    }
                    buildingTable[houseId].uType = HOUSE_TYPE_MERCENARY_GUILD;
                } break;

                case 4:
                    buildingTable[houseId].uAnimationID = atoi(test_string);
                    break;
                case 5:
                    buildingTable[houseId].name = removeQuotes(test_string);
                    break;
                case 6:
                    buildingTable[houseId].pProprieterName = removeQuotes(test_string);
                    break;
                case 7:
                    buildingTable[houseId].pProprieterTitle = removeQuotes(test_string);
                    break;
                case 8:
                    buildingTable[houseId].field_14 = atoi(test_string);
                    break;
                case 9:
                    buildingTable[houseId]._state = atoi(test_string);
                    break;
                case 10:
                    buildingTable[houseId]._rep = atoi(test_string);
                    break;
                case 11:
                    buildingTable[houseId]._per = atoi(test_string);
                    break;
                case 12:
                    buildingTable[houseId].fPriceMultiplier = atof(test_string);
                    break;
                case 13:
                    buildingTable[houseId].flt_24 = atof(test_string);
                    break;
                case 15:
                    buildingTable[houseId].generation_interval_days = atoi(test_string);
                    break;
                case 18:
                    buildingTable[houseId].uOpenTime = atoi(test_string);
                    break;
                case 19:
                    buildingTable[houseId].uCloseTime = atoi(test_string);
                    break;
                case 20:
                    buildingTable[houseId].uExitPicID = atoi(test_string);
                    break;
                case 21:
                    buildingTable[houseId].uExitMapID = static_cast<MapId>(atoi(test_string));
                    break;
                case 22:
                    buildingTable[houseId]._quest_bit = static_cast<QuestBit>(atoi(test_string));
                    break;
                case 23:
                    buildingTable[houseId].pEnterText = removeQuotes(test_string);
                    break;
                }
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 24) && !break_loop);
    }
}
