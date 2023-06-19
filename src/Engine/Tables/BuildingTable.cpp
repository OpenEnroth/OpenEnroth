#include "Engine/Tables/BuildingTable.h"
#include "Engine/Engine.h"
#include "Utility/String.h"

std::array<BuildingDesc, 525> buildingTable;

void initializeBuildings() {
    int i;
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    std::string txtRaw{ engine->_gameResourceManager->getEventsFile("2dEvents.txt").string_view() };
    strtok(txtRaw.data(), "\r");
    strtok(NULL, "\r");

    for (i = 0; i < 525; ++i) {
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
                    if (istarts_with(test_string, "wea")) {
                        buildingTable[i].uType = BUILDING_WEAPON_SHOP;
                        break;
                    }
                    if (istarts_with(test_string, "arm")) {
                        buildingTable[i].uType = BUILDING_ARMOR_SHOP;
                        break;
                    }
                    if (istarts_with(test_string, "mag")) {
                        buildingTable[i].uType = BUILDING_MAGIC_SHOP;
                        break;
                    }
                    if (istarts_with(test_string, "alc")) {
                        buildingTable[i].uType = BUILDING_ALCHEMY_SHOP;
                        break;
                    }
                    if (istarts_with(test_string, "sta")) {
                        buildingTable[i].uType = BUILDING_STABLE;
                        break;
                    }
                    if (istarts_with(test_string, "boa")) {
                        buildingTable[i].uType = BUILDING_BOAT;
                        break;
                    }
                    if (istarts_with(test_string, "tem")) {
                        buildingTable[i].uType = BUILDING_TEMPLE;
                        break;
                    }
                    if (istarts_with(test_string, "tra")) {
                        buildingTable[i].uType = BUILDING_TRAINING_GROUND;
                        break;
                    }
                    if (istarts_with(test_string, "tow")) {
                        buildingTable[i].uType = BUILDING_TOWN_HALL;
                        break;
                    }

                    if (istarts_with(test_string, "tav")) {
                        buildingTable[i].uType = BUILDING_TAVERN;
                        break;
                    }
                    if (istarts_with(test_string, "ban")) {
                        buildingTable[i].uType = BUILDING_BANK;
                        break;
                    }
                    if (istarts_with(test_string, "fir")) {
                        buildingTable[i].uType = BUILDING_FIRE_GUILD;
                        break;
                    }
                    if (istarts_with(test_string, "air")) {
                        buildingTable[i].uType = BUILDING_AIR_GUILD;
                        break;
                    }
                    if (istarts_with(test_string, "wat")) {
                        buildingTable[i].uType = BUILDING_WATER_GUILD;
                        break;
                    }
                    if (istarts_with(test_string, "ear")) {
                        buildingTable[i].uType = BUILDING_EARTH_GUILD;
                        break;
                    }
                    if (istarts_with(test_string, "spi")) {
                        buildingTable[i].uType = BUILDING_SPIRIT_GUILD;
                        break;
                    }
                    if (istarts_with(test_string, "min")) {
                        buildingTable[i].uType = BUILDING_MIND_GUILD;
                        break;
                    }
                    if (istarts_with(test_string, "bod")) {
                        buildingTable[i].uType = BUILDING_BODY_GUILD;
                        break;
                    }
                    if (istarts_with(test_string, "lig")) {
                        buildingTable[i].uType = BUILDING_LIGHT_GUILD;
                        break;
                    }
                    if (istarts_with(test_string, "dar")) {
                        buildingTable[i].uType = BUILDING_DARK_GUILD;
                        break;
                    }
                    if (istarts_with(test_string, "ele")) { // "Element Guild" from mm6
                        buildingTable[i].uType = BUILDING_ELEMENTAL_GUILD;
                        break;
                    }
                    if (istarts_with(test_string, "sel")) {
                        buildingTable[i].uType = BUILDING_SELF_GUILD;
                        break;
                    }
                    if (istarts_with(test_string, "mir")) {
                        buildingTable[i].uType = BUILDING_MIRRORED_PATH_GUILD;
                        break;
                    }
                    if (istarts_with(test_string, "mer")) { // "Thieves Guild" from mm6
                        buildingTable[i].uType = BUILDING_TOWN_HALL; //TODO: Is this right and not Merc Guild (18)?
                        break;
                    }
                    buildingTable[i].uType = BUILDING_MERCENARY_GUILD;
                } break;

                case 4:
                    buildingTable[i].uAnimationID = atoi(test_string);
                    break;
                case 5:
                    buildingTable[i].pName = removeQuotes(test_string);
                    break;
                case 6:
                    buildingTable[i].pProprieterName = removeQuotes(test_string);
                    break;
                case 7:
                    buildingTable[i].pProprieterTitle = removeQuotes(test_string);
                    break;
                case 8:
                    buildingTable[i].field_14 = atoi(test_string);
                    break;
                case 9:
                    buildingTable[i]._state = atoi(test_string);
                    break;
                case 10:
                    buildingTable[i]._rep = atoi(test_string);
                    break;
                case 11:
                    buildingTable[i]._per = atoi(test_string);
                    break;
                case 12:
                    buildingTable[i].fPriceMultiplier = atof(test_string);
                    break;
                case 13:
                    buildingTable[i].flt_24 = atof(test_string);
                    break;
                case 15:
                    buildingTable[i].generation_interval_days = atoi(test_string);
                    break;
                case 18:
                    buildingTable[i].uOpenTime = atoi(test_string);
                    break;
                case 19:
                    buildingTable[i].uCloseTime = atoi(test_string);
                    break;
                case 20:
                    buildingTable[i].uExitPicID = atoi(test_string);
                    break;
                case 21:
                    buildingTable[i].uExitMapID = atoi(test_string);
                    break;
                case 22:
                    buildingTable[i]._quest_bit = atoi(test_string);
                    break;
                case 23:
                    buildingTable[i].pEnterText = removeQuotes(test_string);
                    break;
                }
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 24) && !break_loop);
    }
}
