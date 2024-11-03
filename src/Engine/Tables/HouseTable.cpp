#include "HouseTable.h"

#include <cstring>
#include <string>

#include "Engine/Data/HouseEnumFunctions.h"

#include "Utility/Memory/Blob.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Transformations.h"

IndexedArray<HouseData, HOUSE_FIRST, HOUSE_LAST> houseTable;

void initializeHouses(const Blob &houses) {
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    std::string txtRaw(houses.string_view());
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
                        houseTable[houseId].uType = HOUSE_TYPE_WEAPON_SHOP;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "arm")) {
                        houseTable[houseId].uType = HOUSE_TYPE_ARMOR_SHOP;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "mag")) {
                        houseTable[houseId].uType = HOUSE_TYPE_MAGIC_SHOP;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "alc")) {
                        houseTable[houseId].uType = HOUSE_TYPE_ALCHEMY_SHOP;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "sta")) {
                        houseTable[houseId].uType = HOUSE_TYPE_STABLE;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "boa")) {
                        houseTable[houseId].uType = HOUSE_TYPE_BOAT;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "tem")) {
                        houseTable[houseId].uType = HOUSE_TYPE_TEMPLE;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "tra")) {
                        houseTable[houseId].uType = HOUSE_TYPE_TRAINING_GROUND;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "tow")) {
                        houseTable[houseId].uType = HOUSE_TYPE_TOWN_HALL;
                        break;
                    }

                    if (ascii::noCaseStartsWith(test_string, "tav")) {
                        houseTable[houseId].uType = HOUSE_TYPE_TAVERN;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "ban")) {
                        houseTable[houseId].uType = HOUSE_TYPE_BANK;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "fir")) {
                        houseTable[houseId].uType = HOUSE_TYPE_FIRE_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "air")) {
                        houseTable[houseId].uType = HOUSE_TYPE_AIR_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "wat")) {
                        houseTable[houseId].uType = HOUSE_TYPE_WATER_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "ear")) {
                        houseTable[houseId].uType = HOUSE_TYPE_EARTH_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "spi")) {
                        houseTable[houseId].uType = HOUSE_TYPE_SPIRIT_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "min")) {
                        houseTable[houseId].uType = HOUSE_TYPE_MIND_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "bod")) {
                        houseTable[houseId].uType = HOUSE_TYPE_BODY_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "lig")) {
                        houseTable[houseId].uType = HOUSE_TYPE_LIGHT_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "dar")) {
                        houseTable[houseId].uType = HOUSE_TYPE_DARK_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "ele")) { // "Element Guild" from mm6
                        houseTable[houseId].uType = HOUSE_TYPE_ELEMENTAL_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "sel")) {
                        houseTable[houseId].uType = HOUSE_TYPE_SELF_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "mir")) {
                        houseTable[houseId].uType = HOUSE_TYPE_MIRRORED_PATH_GUILD;
                        break;
                    }
                    if (ascii::noCaseStartsWith(test_string, "mer")) { // "Thieves Guild" from mm6
                        houseTable[houseId].uType = HOUSE_TYPE_TOWN_HALL; //TODO: Is this right and not Merc Guild (18)?
                        break;
                    }
                    houseTable[houseId].uType = HOUSE_TYPE_MERCENARY_GUILD;
                } break;

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
