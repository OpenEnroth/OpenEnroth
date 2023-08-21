#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <unordered_map>
#include <vector>
#include <utility>
#include <algorithm>
#include <exception>
#include <string>

#include "CodeGenOptions.h"
#include "Application/GameStarter.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/GameResourceManager.h"
#include "Engine/MapInfo.h"
#include "Library/Lod/LodReader.h"
#include "Utility/DataPath.h"
#include "Utility/Exception.h"
#include "Utility/String.h"
#include "Engine/Objects/ItemEnums.h"
#include "Engine/Objects/Items.h"
#include "Utility/IndexedArray.h"
#include "Utility/Segment.h"
#include "fmt/core.h"

enum class MAP_TYPE : uint32_t;

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

    std::unordered_map<std::string, int> countByName;
    std::unordered_map<std::string, int> indexByName;
    std::vector<std::pair<std::string, std::string>> items;

    items.emplace_back("NULL", "");

    for(ITEM_TYPE i : itemTable.pItems.indices()) {
        const ItemDesc &desc = itemTable.pItems[i];
        std::string icon = desc.iconName;
        std::string name = desc.name;
        std::string description = desc.pDescription;

        if (icon.empty() || icon == "null") {
            items.emplace_back("", "Unused.");
            continue;
        }

        std::string enumName = toUpperCaseEnum(name);

        if (enumName == "EMPTY_MESSAGE_SCROLL" || enumName == "NAME_OF_MESSAGE") {
            items.emplace_back("", "Empty scroll placeholder, unused.");
            continue;
        }

        if (enumName == "NEWNAME_KEY") {
            items.emplace_back("", "Key placeholder, unused.");
            continue;
        }

        if (contains(enumName, "PLACEHOLDER") || contains(enumName, "SEALED_LETTER")) {
            items.emplace_back("", name + ", unused.");
            continue;
        }

        if (contains(enumName, "ORDERS_FROM_SNERGLE")) {
            items.emplace_back("", name + ", unused remnant from MM6.");
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

        if (indexByName.contains(enumName)) {
            int count = ++countByName[enumName];
            if (count == 2)
                items[indexByName[enumName]].first = enumName + "_1";

            enumName = enumName + "_" + std::to_string(count);
        } else {
            indexByName[enumName] = items.size();
            countByName[enumName] = 1;
        }

        items.emplace_back(enumName, "");
    }

    for (size_t i = 0; i < items.size(); i++) {
        if (!items[i].first.empty()) {
            fmt::print("    ITEM_{} = {},\n", items[i].first, i);
        } else {
            fmt::print("    ITEM_{} = {}, // {}\n", i, i, items[i].second);
        }
    }

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

    std::vector<std::string> maps;

    maps.emplace_back("INVALID");
    for (MAP_TYPE i : mapStats.pInfos.indices())
        maps.emplace_back(mapIdEnumName(mapStats.pInfos[i]));

    for (size_t i = 0; i < maps.size(); i++)
        fmt::print("    MAP_{} = {},\n", maps[i], i);

    return 0;
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

        auto pos = std::find_if(mapStats.pInfos.begin(), mapStats.pInfos.end(), [&] (const MapInfo &mapInfo) {
            return toLower(mapInfo.pFilename) == toLower(fileName);
        });
        if (pos == mapStats.pInfos.end())
            throw Exception("Unrecognized map '{}'", fileName);

        fmt::println("    {{MAP_{}, {}}},", mapIdEnumName(*pos), i);
    }

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
        default:
            assert(false);
            return 1;
        }
    } catch (const std::exception &e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
}
