#include "CodeGenOptions.h"

#include <set>
#include <unordered_map>
#include <vector>
#include <utility>

#include "Application/GameStarter.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/BuildingTable.h"
#include "Engine/Events/EventMap.h"
#include "Engine/GameResourceManager.h"
#include "Engine/MapInfo.h"

#include "Library/Lod/LodReader.h"
#include "Library/Serialization/EnumSerialization.h"

#include "Utility/Format.h"
#include "Utility/DataPath.h"
#include "Utility/Exception.h"
#include "Utility/String.h"

#include "CodeGenEnums.h"
#include "CodeGenMap.h"

// TODO(captainurist): use std::string::contains once Android have full C++23 support.
static auto contains = [](const std::string &haystack, const std::string &needle) {
    return haystack.find(needle) != std::string::npos;
};

static std::string toUpperCaseEnum(const std::string &string) {
    std::string result;
    for (char c : string) {
        if (isalnum(c)) {
            result += static_cast<char>(toupper(c));
        } else if (isspace(c) || c == '/' || c == '-') {
            if (!result.ends_with('_'))
                result += '_';
        }
    }
    return result;
}

int runItemIdCodeGen(CodeGenOptions options, GameResourceManager *resourceManager) {
    ItemTable itemTable;
    itemTable.Initialize(resourceManager);

    CodeGenMap map;
    map.insert(ITEM_NULL, "NULL", "");

    for(ItemId i : itemTable.pItems.indices()) {
        const ItemDesc &desc = itemTable.pItems[i];
        std::string icon = desc.iconName;
        std::string name = desc.name;
        std::string description = desc.pDescription;

        if (icon.empty() || icon == "null") {
            map.insert(i, "", "Unused.");
            continue;
        }

        std::string enumName = toUpperCaseEnum(name);

        if (enumName == "EMPTY_MESSAGE_SCROLL" || enumName == "NAME_OF_MESSAGE") {
            map.insert(i, "", "Empty scroll placeholder, unused.");
            continue;
        }

        if (enumName == "NEWNAME_KEY") {
            map.insert(i, "", "Key placeholder, unused.");
            continue;
        }

        if (contains(enumName, "PLACEHOLDER") || contains(enumName, "SEALED_LETTER")) {
            map.insert(i, "", name + ", unused.");
            continue;
        }

        if (contains(enumName, "ORDERS_FROM_SNERGLE")) {
            map.insert(i, "", name + ", unused remnant from MM6.");
            continue;
        }

        if (enumName == "LICH_JAR") {
            if (contains(description, "Empty")) {
                enumName += "_EMPTY";
            } else {
                enumName += "_FULL";
            }
        }

        if (enumName == "THE_PERFECT_BOW")
            if (!contains(description, "off-balance"))
                enumName += "_FIXED";

        if (desc.uEquipType == EQUIP_REAGENT) {
            enumName = "REAGENT_" + enumName;
        } else if (desc.uEquipType == EQUIP_POTION) {
            if (!enumName.starts_with("POTION_"))
                enumName = "POTION_" + enumName;
            if (enumName.ends_with("_POTION"))
                enumName = enumName.substr(0, enumName.size() - 7);
        } else if (desc.uEquipType == EQUIP_SPELL_SCROLL) {
            enumName = "SCROLL_" + enumName;
        } else if (desc.uEquipType == EQUIP_BOOK) {
            enumName = "SPELLBOOK_" + enumName;
        } else if (desc.uEquipType == EQUIP_MESSAGE_SCROLL) {
            if (enumName.ends_with("_RECIPE")) {
                enumName = "RECIPE_" + enumName.substr(0, enumName.size() - 7);
            } else if (!enumName.starts_with("MESSAGE_")) {
                enumName = "MESSAGE_" + enumName;
            }
        } else if (desc.uEquipType == EQUIP_GOLD) {
            if (description == "A small pile of gold coins.") {
                enumName = "GOLD_SMALL";
            } else if (description == "A pile of gold coins.") {
                enumName = "GOLD_MEDIUM";
            } else if (description == "A large pile of gold coins.") {
                enumName = "GOLD_LARGE";
            } else {
                throw Exception("Unrecognized gold pile description '{}'", description);
            }
        } else if (desc.uEquipType == EQUIP_GEM) {
            enumName = "GEM_" + enumName;
        }

        if (desc.uMaterial == MATERIAL_ARTIFACT) {
            enumName = "ARTIFACT_" + enumName;
        } else if (desc.uMaterial == MATERIAL_RELIC) {
            enumName = "RELIC_" + enumName;
        } else if (desc.uMaterial == MATERIAL_SPECIAL) {
            enumName = "RARE_" + enumName;
        } else if (description.starts_with("Quest")) {
            enumName = "QUEST_" + enumName;
        }

        map.insert(i, enumName, "");
    }

    map.dump(stdout, "ITEM_");

    return 0;
}

std::string mapIdEnumName(const MapInfo &mapInfo) {
    std::string result = toUpperCaseEnum(mapInfo.pName);
    if (result.starts_with("THE_"))
        result = result.substr(4);
    return result;
}

int runMapIdCodeGen(CodeGenOptions options, GameResourceManager *resourceManager) {
    MapStats mapStats;
    mapStats.Initialize(resourceManager->getEventsFile("MapStats.txt"));

    CodeGenMap map;
    map.insert(MAP_INVALID, "INVALID", "");

    for (MapId i : mapStats.pInfos.indices())
        map.insert(i, mapIdEnumName(mapStats.pInfos[i]), "");

    map.dump(stdout, "MAP_");
    return 0;
}

const MapInfo &mapInfoByFileName(const MapStats &mapStats, const std::string &fileName) {
    auto pos = std::find_if(mapStats.pInfos.begin(), mapStats.pInfos.end(), [&] (const MapInfo &mapInfo) {
        return toLower(mapInfo.pFilename) == toLower(fileName);
    });
    if (pos == mapStats.pInfos.end())
        throw Exception("Unrecognized map '{}'", fileName);
    return *pos;
}

int runBeaconsCodeGen(CodeGenOptions options, GameResourceManager *resourceManager) {
    MapStats mapStats;
    mapStats.Initialize(resourceManager->getEventsFile("MapStats.txt"));

    LodReader gamesLod(makeDataPath("data", "games.lod"));
    std::vector<std::string> fileNames = gamesLod.ls();

    for (size_t i = 0; i < fileNames.size(); i++) {
        const std::string &fileName = fileNames[i];
        if (!fileName.ends_with(".odm") && !fileName.ends_with(".blv"))
            continue; // Not a level file.

        fmt::println("    {{MAP_{}, {}}},", mapIdEnumName(mapInfoByFileName(mapStats, fileName)), i);
    }

    return 0;
}

int runHouseIdCodeGen(CodeGenOptions options, GameResourceManager *resourceManager) {
    MapStats mapStats;
    mapStats.Initialize(resourceManager->getEventsFile("MapStats.txt"));

    initializeBuildings(resourceManager->getEventsFile("2dEvents.txt"));
    // ^ Initializes buildingTable.

    std::unordered_map<HouseId, std::set<std::string>> mapNamesByHouseId; // Only arbiter exists on two maps.

    LodReader gamesLod(makeDataPath("data", "games.lod"));
    for (const std::string &fileName : gamesLod.ls()) {
        if (!fileName.ends_with(".odm") && !fileName.ends_with(".blv"))
            continue; // Not a level file.

        std::string mapName = mapIdEnumName(mapInfoByFileName(mapStats, fileName));
        EventMap eventMap = EventMap::load(resourceManager->getEventsFile(fileName.substr(0, fileName.size() - 4) + ".evt"));

        for (const EventTrigger &trigger : eventMap.enumerateTriggers(EVENT_SpeakInHouse)) {
            HouseId houseId = eventMap.event(trigger.eventId, trigger.eventStep).data.house_id;
            if (houseId == HOUSE_INVALID)
                throw Exception("Invalid house id encountered in house event");
            mapNamesByHouseId[houseId].insert(mapName);
        }

        for (const EventTrigger &trigger : eventMap.enumerateTriggers(EVENT_MoveToMap)) {
            HouseId houseId = eventMap.event(trigger.eventId, trigger.eventStep).data.move_map_descr.house_id;
            if (houseId != HOUSE_INVALID)
                mapNamesByHouseId[houseId].insert(mapName);
        }
    }

    CodeGenMap map;
    map.insert(HOUSE_INVALID, "INVALID", "");

    for (HouseId i : buildingTable.indices()) {
        const BuildingDesc &desc = buildingTable[i];
        bool hasMap = mapNamesByHouseId.contains(i);
        std::string mapName;
        if (hasMap)
            mapName = fmt::format("{}", fmt::join(mapNamesByHouseId[i], "_"));

        if (i == HOUSE_JAIL) {
            map.insert(i, "JAIL", "");
        } else if (desc.uType == BUILDING_INVALID && hasMap) {
            map.insert(i, "", fmt::format("Used in MAP_{} but invalid, hmm...", mapName));
        } else if (desc.uType == BUILDING_INVALID) {
            map.insert(i, "", "Unused.");
        } else if (!hasMap && !desc.pName.empty()) {
            map.insert(i, "", fmt::format("Unused {} named \"{}\".", toString(desc.uType), desc.pName));
        } else if (!hasMap) {
            map.insert(i, "", "Unused.");
        } else if (toUpperCaseEnum(desc.pName) == fmt::format("HOUSE_{}", std::to_underlying(i))) {
            map.insert(i, "", fmt::format("Used in MAP_{}, named \"{}\", looks totally like a placeholder...", mapName, desc.pName));
        } else if (desc.uType == BUILDING_HOUSE || desc.uType == BUILDING_MERCENARY_GUILD) {
            map.insert(i, fmt::format("{}_{}", mapName, toUpperCaseEnum(desc.pName)), "");
        } else {
            map.insert(i, fmt::format("{}_{}", toString(desc.uType), mapName), fmt::format("\"{}\".", trim(desc.pName)));
        }
    }

    map.dump(stdout, "HOUSE_");
    return 0;
}

int platformMain(int argc, char **argv) {
    try {
        CodeGenOptions options = CodeGenOptions::parse(argc, argv);
        if (options.helpPrinted)
            return 1;

        GameStarter starter(options);

        GameResourceManager resourceManager;
        resourceManager.openGameResources();

        switch (options.subcommand) {
        case CodeGenOptions::SUBCOMMAND_ITEM_ID: return runItemIdCodeGen(std::move(options), &resourceManager);
        case CodeGenOptions::SUBCOMMAND_MAP_ID: return runMapIdCodeGen(std::move(options), &resourceManager);
        case CodeGenOptions::SUBCOMMAND_BEACON_MAPPING: return runBeaconsCodeGen(std::move(options), &resourceManager);
        case CodeGenOptions::SUBCOMMAND_HOUSE_ID: return runHouseIdCodeGen(std::move(options), &resourceManager);
        default:
            assert(false);
            return 1;
        }
    } catch (const std::exception &e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
}
