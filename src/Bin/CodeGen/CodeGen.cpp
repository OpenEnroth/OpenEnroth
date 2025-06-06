#include "CodeGenOptions.h"

#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>
#include <map>
#include <regex>
#include <string>

#include "Application/Startup/GameStarter.h"

#include "Engine/Data/HouseEnumFunctions.h"
#include "Engine/Components/Random/EngineRandomComponent.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/HouseTable.h"
#include "Engine/Evt/EvtProgram.h"
#include "Engine/Random/Random.h"
#include "Engine/Objects/DecorationEnums.h"
#include "Engine/Objects/DecorationList.h"
#include "Engine/Objects/Monsters.h"
#include "Engine/Objects/MonsterEnumFunctions.h"
#include "Engine/Snapshots/TableSerialization.h"
#include "Engine/GameResourceManager.h"
#include "Engine/MapInfo.h"
#include "Engine/EngineFileSystem.h"
#include "Engine/mm7_data.h"

#include "GUI/UI/Houses/TownHall.h"

#include "Library/Lod/LodReader.h"
#include "Library/Serialization/EnumSerialization.h"

#include "Utility/String/Ascii.h"
#include "Utility/String/Format.h"
#include "Utility/Exception.h"
#include "Utility/UnicodeCrt.h"
#include "Utility/String/Transformations.h"

#include "CodeGenEnums.h"
#include "CodeGenMap.h"

int runItemIdCodeGen(const CodeGenOptions &options, GameResourceManager *resourceManager) {
    ItemTable itemTable;
    itemTable.Initialize(resourceManager);

    CodeGenMap map;
    map.insert(ITEM_NULL, "NULL", "");

    for(ItemId i : itemTable.items.indices()) {
        const ItemData &desc = itemTable.items[i];
        std::string icon = desc.iconName;
        std::string name = desc.name;
        std::string description = desc.description;

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

        if (enumName.contains("PLACEHOLDER") || enumName.contains("SEALED_LETTER")) {
            map.insert(i, "", name + ", unused.");
            continue;
        }

        if (enumName.contains("ORDERS_FROM_SNERGLE")) {
            map.insert(i, "", name + ", unused remnant from MM6.");
            continue;
        }

        if (enumName == "LICH_JAR") {
            if (description.contains("Empty")) {
                enumName += "_EMPTY";
            } else {
                enumName += "_FULL";
            }
        }

        if (enumName == "THE_PERFECT_BOW")
            if (!description.contains("off-balance"))
                enumName += "_FIXED";

        // Shorten enum names for letters.
        if (enumName.starts_with("LETTER_FROM") && enumName.contains("_TO_"))
            enumName = enumName.substr(0, enumName.find("_TO_"));

        if (desc.type == ITEM_TYPE_REAGENT) {
            enumName = "REAGENT_" + enumName;
        } else if (desc.type == ITEM_TYPE_POTION) {
            if (!enumName.starts_with("POTION_"))
                enumName = "POTION_" + enumName;
            if (enumName.ends_with("_POTION"))
                enumName = enumName.substr(0, enumName.size() - 7);
        } else if (desc.type == ITEM_TYPE_SPELL_SCROLL) {
            enumName = "SCROLL_" + enumName;
        } else if (desc.type == ITEM_TYPE_BOOK) {
            enumName = "SPELLBOOK_" + enumName;
        } else if (desc.type == ITEM_TYPE_MESSAGE_SCROLL) {
            if (enumName.ends_with("_RECIPE")) {
                enumName = "RECIPE_" + enumName.substr(0, enumName.size() - 7);
            } else if (!enumName.starts_with("MESSAGE_")) {
                enumName = "MESSAGE_" + enumName;
            }
        } else if (desc.type == ITEM_TYPE_GOLD) {
            if (description == "A small pile of gold coins.") {
                enumName = "GOLD_SMALL";
            } else if (description == "A pile of gold coins.") {
                enumName = "GOLD_MEDIUM";
            } else if (description == "A large pile of gold coins.") {
                enumName = "GOLD_LARGE";
            } else {
                throw Exception("Unrecognized gold pile description '{}'", description);
            }
        } else if (desc.type == ITEM_TYPE_GEM) {
            enumName = "GEM_" + enumName;
        }

        if (desc.rarity == RARITY_ARTIFACT) {
            enumName = "ARTIFACT_" + enumName;
        } else if (desc.rarity == RARITY_RELIC) {
            enumName = "RELIC_" + enumName;
        } else if (desc.rarity == RARITY_SPECIAL) {
            enumName = "SPECIAL_" + enumName;
        } else if (description.starts_with("Quest")) {
            enumName = "QUEST_" + enumName;
        }

        map.insert(i, enumName, "");
    }

    map.dump(stdout, "ITEM_");

    return 0;
}

std::string mapIdEnumName(const MapInfo &mapInfo) {
    std::string result = toUpperCaseEnum(mapInfo.name);
    if (result.starts_with("THE_"))
        result = result.substr(4);
    return result;
}

int runMapIdCodeGen(const CodeGenOptions &options, GameResourceManager *resourceManager) {
    MapStats mapStats;
    mapStats.Initialize(resourceManager->getEventsFile("MapStats.txt"));

    CodeGenMap map;
    map.insert(MAP_INVALID, "INVALID", "");

    for (MapId i : mapStats.pInfos.indices())
        map.insert(i, mapIdEnumName(mapStats.pInfos[i]), "");

    map.dump(stdout, "MAP_");
    return 0;
}

const MapInfo &mapInfoByFileName(const MapStats &mapStats, std::string_view fileName) {
    auto pos = std::find_if(mapStats.pInfos.begin(), mapStats.pInfos.end(), [&] (const MapInfo &mapInfo) {
        return ascii::noCaseEquals(mapInfo.fileName, fileName);
    });
    if (pos == mapStats.pInfos.end())
        throw Exception("Unrecognized map '{}'", fileName);
    return *pos;
}

int runBeaconsCodeGen(const CodeGenOptions &options, GameResourceManager *resourceManager) {
    MapStats mapStats;
    mapStats.Initialize(resourceManager->getEventsFile("MapStats.txt"));

    LodReader gamesLod(dfs->read("data/games.lod"));
    std::vector<std::string> fileNames = gamesLod.ls();

    for (size_t i = 0; i < fileNames.size(); i++) {
        const std::string &fileName = fileNames[i];
        if (!fileName.ends_with(".odm") && !fileName.ends_with(".blv"))
            continue; // Not a level file.

        fmt::println("    {{MAP_{}, {}}},", mapIdEnumName(mapInfoByFileName(mapStats, fileName)), i);
    }

    return 0;
}

int runHouseIdCodeGen(const CodeGenOptions &options, GameResourceManager *resourceManager) {
    MapStats mapStats;
    mapStats.Initialize(resourceManager->getEventsFile("MapStats.txt"));

    initializeHouses(resourceManager->getEventsFile("2dEvents.txt"));
    // ^ Initializes houseTable.

    std::unordered_map<HouseId, std::set<std::string>> mapNamesByHouseId; // Only arbiter exists on two maps.

    LodReader gamesLod(dfs->read("data/games.lod"));
    for (const std::string &fileName : gamesLod.ls()) {
        if (!fileName.ends_with(".odm") && !fileName.ends_with(".blv"))
            continue; // Not a level file.

        std::string mapName = mapIdEnumName(mapInfoByFileName(mapStats, fileName));
        EvtProgram eventMap = EvtProgram::load(resourceManager->getEventsFile(fileName.substr(0, fileName.size() - 4) + ".evt"));

        for (const EventTrigger &trigger : eventMap.enumerateTriggers(EVENT_SpeakInHouse)) {
            HouseId houseId = eventMap.instruction(trigger.eventId, trigger.eventStep).data.house_id;
            if (houseId == HOUSE_INVALID)
                throw Exception("Invalid house id encountered in house event");
            mapNamesByHouseId[houseId].insert(mapName);
        }

        for (const EventTrigger &trigger : eventMap.enumerateTriggers(EVENT_MoveToMap)) {
            HouseId houseId = eventMap.instruction(trigger.eventId, trigger.eventStep).data.move_map_descr.house_id;
            if (houseId != HOUSE_INVALID)
                mapNamesByHouseId[houseId].insert(mapName);
        }
    }

    CodeGenMap map;
    map.insert(HOUSE_INVALID, "INVALID", "");

    for (HouseId i : houseTable.indices()) {
        const HouseData &desc = houseTable[i];
        bool hasMap = mapNamesByHouseId.contains(i);
        std::string mapName;
        if (hasMap)
            mapName = fmt::format("{}", fmt::join(mapNamesByHouseId[i], "_"));

        if (i == HOUSE_JAIL) {
            map.insert(i, "JAIL", "");
        } else if (desc.uType == HOUSE_TYPE_INVALID && hasMap) {
            map.insert(i, "", fmt::format("Used in MAP_{} but invalid, hmm...", mapName));
        } else if (desc.uType == HOUSE_TYPE_INVALID) {
            map.insert(i, "", "Unused.");
        } else if (!hasMap && !desc.name.empty()) {
            map.insert(i, "", fmt::format("Unused {} named \"{}\".", toString(desc.uType), desc.name));
        } else if (!hasMap) {
            map.insert(i, "", "Unused.");
        } else if (toUpperCaseEnum(desc.name) == fmt::format("HOUSE_{}", std::to_underlying(i))) {
            map.insert(i, "", fmt::format("Used in MAP_{}, named \"{}\", looks totally like a placeholder...", mapName, desc.name));
        } else if (desc.uType == HOUSE_TYPE_HOUSE || desc.uType == HOUSE_TYPE_MERCENARY_GUILD) {
            map.insert(i, fmt::format("{}_{}", mapName, toUpperCaseEnum(desc.name)), "");
        } else {
            map.insert(i, fmt::format("{}_{}", toString(desc.uType), mapName), fmt::format("\"{}\".", trim(desc.name)));
        }
    }

    map.dump(stdout, "HOUSE_");
    return 0;
}

MonsterStats loadMonsterStats(GameResourceManager *resourceManager) {
    TriBlob dmonBlobs;
    dmonBlobs.mm7 = resourceManager->getEventsFile("dmonlist.bin");

    pMonsterList = new MonsterList;
    deserialize(dmonBlobs, pMonsterList);

    MonsterStats result;
    result.Initialize(resourceManager->getEventsFile("monsters.txt"));
    return result;
}

std::string cleanupMonsterIdEnumName(std::string enumName) {
    for (const char *prefix : {"ZBLASTERGUY", "ZULTRA_DRAGON", })
        if (enumName.starts_with(prefix))
            enumName = enumName.substr(1);

    for (const char *prefix : {"ZCAT", "ZCHICKEN", "ZDOG", "ZRAT"})
        if (enumName.starts_with(prefix))
            enumName = "UNUSED_" + enumName.substr(1);

    enumName = replaceAll(enumName, "MALEA", "MALE_A");
    enumName = replaceAll(enumName, "MALEB", "MALE_B");
    enumName = replaceAll(enumName, "MALEC", "MALE_C");

    return enumName;
}

int runMonsterIdCodeGen(const CodeGenOptions &options, GameResourceManager *resourceManager) {
    MonsterStats monsterStats = loadMonsterStats(resourceManager);

    CodeGenMap map;
    map.insert(MONSTER_INVALID, "INVALID", "");

    for (const MonsterId i : allMonsters()) {
        const MonsterDesc &desc = pMonsterList->monsters[i];
        const MonsterInfo &info = monsterStats.infos[i];
        std::string enumName = cleanupMonsterIdEnumName(toUpperCaseEnum(desc.monsterName));

        std::string comment = info.name;
        if (comment == "peasant")
            comment = "Peasant";
        if (!comment.empty())
            comment = fmt::format("\"{}\".", trim(comment));

        map.insert(i, enumName, comment);
    }

    map.dump(stdout, "MONSTER_");
    return 0;
}

std::string cleanupMonsterTypeEnumName(std::string enumName) {
    enumName = cleanupMonsterIdEnumName(enumName);

    if (enumName.ends_with("_A")) {
        enumName.resize(enumName.size() - 2);
    } else if (!enumName.empty()) {
        throw Exception("Invalid monster id name");
    }

    return enumName;
}

int runMonsterTypeCodeGen(const CodeGenOptions &options, GameResourceManager *resourceManager) {
    MonsterStats monsterStats = loadMonsterStats(resourceManager);

    CodeGenMap map;
    map.insert(MONSTER_TYPE_INVALID, "INVALID", "");

    int counter = 0;
    for (const MonsterId i : allMonsters()) {
        if (++counter % 3 != 1)
            continue;

        const MonsterDesc &desc = pMonsterList->monsters[i];
        std::string enumName = cleanupMonsterTypeEnumName(toUpperCaseEnum(desc.monsterName));

        map.insert(monsterTypeForMonsterId(i), enumName, "");
    }

    map.dump(stdout, "MONSTER_TYPE_");
    return 0;
}

int runBountyHuntCodeGen(const CodeGenOptions &options, GameResourceManager *resourceManager) {
    // Fill bounty hunt map.
    EngineRandomComponent randomComponent;
    randomComponent.setType(RANDOM_ENGINE_SEQUENTIAL);

    IndexedArray<std::unordered_set<MonsterId>, HOUSE_FIRST_TOWN_HALL, HOUSE_LAST_TOWN_HALL> monstersByTownHall;
    for (const HouseId townHall : allTownhallHouses()) {
        grng->seed(0);
        while(true) {
            MonsterId monsterId = GUIWindow_TownHall::randomMonsterForHunting(townHall);
            if (!monstersByTownHall[townHall].insert(monsterId).second)
                break;
        }
    }

    // Invert the map.
    std::unordered_map<MonsterId, std::unordered_set<HouseId>> townHallsByMonster;
    for (const HouseId townHall : allTownhallHouses())
        for (const MonsterId monsterId : monstersByTownHall[townHall])
            townHallsByMonster[monsterId].insert(townHall);

    // Reduce the map to monster types & check that it's actually reducible.
    std::unordered_map<MonsterType, std::unordered_set<HouseId>> townHallsByMonsterType;
    for (const MonsterId monsterId : allMonsters()) {
        MonsterType monsterType = monsterTypeForMonsterId(monsterId);
        if (townHallsByMonsterType.contains(monsterType) && townHallsByMonsterType[monsterType] != townHallsByMonster[monsterId])
            throw Exception("Invalid bounty hunt record");

        townHallsByMonsterType[monsterType] = townHallsByMonster[monsterId];
    }

    // Prepare output table.
    std::vector<std::array<std::string, 8>> table;
    for (const MonsterType monsterType : allMonsterTypes()) {
        auto &line = table.emplace_back();
        line[0] = fmt::format("{{{}, ", toString(monsterType));
        line[1] = "{";
        for (const HouseId townHall : townHallsByMonsterType[monsterType])
            line[2 + std::to_underlying(townHall) - std::to_underlying(HOUSE_FIRST_TOWN_HALL)] = toString(townHall) + ", ";
        for (size_t i = 6; i >= 2; i--) {
            if (!line[i].empty()) {
                line[i].resize(line[i].size() - 2); // Drop the last ", ".
                break;
            }
        }
        line[7] = "}},";
    }

    // Dump!
    dumpAligned(stdout, "    ", table);
    return 0;
}

int runMusicCodeGen(const CodeGenOptions &options, GameResourceManager *resourceManager) {
    MapStats mapStats;
    mapStats.Initialize(resourceManager->getEventsFile("MapStats.txt"));

    std::map<MusicId, std::vector<std::string>> mapNamesByMusicId, mapEnumNamesByMusicId;
    for (const MapInfo &info : mapStats.pInfos) {
        mapNamesByMusicId[info.musicId].push_back(info.name);
        mapEnumNamesByMusicId[info.musicId].push_back(mapIdEnumName(info));
    }

    CodeGenMap map;
    map.insert(MUSIC_INVALID, "INVALID", "");

    for (const auto &[musicId, mapEnumNames] : mapEnumNamesByMusicId) {
        if (mapEnumNames.size() <= 3) {
            map.insert(musicId, fmt::format("{}", fmt::join(mapEnumNames, "_")), "");
        } else if (musicId == MUSIC_DUNGEON) {
            map.insert(musicId, "DUNGEON", "Most of the game dungeons.");
        } else if (musicId == MUSIC_BARROWS) {
            map.insert(musicId, "BARROWS", "Barrows I-XV & Zokarr's Tomb.");
        } else {
            std::string comment = fmt::format("{}.", fmt::join(mapNamesByMusicId[musicId], ", "));

            if (musicId == MUSIC_CASTLE_HARMONDALE) {
                map.insert(musicId, "CASTLE_HARMONDALE", comment);
            } else if (musicId == MUSIC_TEMPLES) {
                map.insert(musicId, "TEMPLES", comment);
            } else if (musicId == MUSIC_ENDGAME_DUNGEON) {
                map.insert(musicId, "ENDGAME_DUNGEON", comment);
            } else {
                throw Exception("Unhandled music id value.");
            }
        }
    }

    map.dump(stdout, "MUSIC_");
    return 0;
}

int runDecorationsCodegen(const CodeGenOptions &options, GameResourceManager *resourceManager) {
    CodeGenMap map;
    std::regex tailRegex("^([A-Za-z ]*)([0-9]+)([a-zA-Z]?)$");

    // Decoration naming & numbering is very weird, and there is no sane approach to naming except just using the
    // id values as a suffix. So this is what we're doing here.

    map.insert(DECORATION_NULL, "NULL", "");
    for (size_t index = 1; index < pDecorationList->pDecorations.size(); index++) {
        DecorationId i = static_cast<DecorationId>(index);
        const DecorationDesc& dd = pDecorationList->pDecorations[index];

        if (dd.name.empty()) {
            map.insert(i, "", "Unused.");
            continue;
        }

        std::string enumName;
        if (dd.name == "fount1") {
            enumName = "FOUNTAIN";
        } else if (dd.name.starts_with("dec")) {
            enumName = dd.type;
        } else {
            enumName = dd.name;
        }
        std::smatch match;
        if (std::regex_search(enumName, match, tailRegex))
            enumName = match[1].str();
        enumName = fmt::format("{}_{}", enumName, index);

        std::string description =  dd.name + ", " + dd.type;
        if (dd.uLightRadius)
            description += fmt::format(", light_r={}", dd.uLightRadius);
        if (dd.uColoredLight.r + dd.uColoredLight.g + dd.uColoredLight.b > 0)
            description += fmt::format(", light_c=#{:02x}{:02x}{:02x}", dd.uColoredLight.r, dd.uColoredLight.g, dd.uColoredLight.b );
        if (dd.uSoundID != SOUND_Invalid)
            description += fmt::format(", snd={}", std::to_underlying(dd.uSoundID));

        map.insert(i, toUpperCaseEnum(enumName), description);
    }
    map.dump(stdout, "DECORATION_");
    return 0;
}

int runSpeechPortraitsCodegen(const CodeGenOptions &options, GameResourceManager *resourceManager) {
    std::vector<std::array<std::string, 7>> table;
    for (CharacterSpeech speech : portraitVariants.indices()) {
        auto &line = table.emplace_back();
        line[0] = fmt::format("{{{}, ", toString(speech));
        line[1] = "{";
        for (int i = 2; auto portrait : portraitVariants[speech])
            line[i++] = toString(portrait) + ", ";
        line[6].pop_back();
        line[6].pop_back(); // Drop the last ", ".
        line[6] += "}},";
    }

    // Dump!
    dumpAligned(stdout, "    ", table);
    return 0;
}

int runLstrCodegen(const CodeGenOptions &options, GameResourceManager *resourceManager) {
    CodeGenMap map;

    std::string txt = std::string(resourceManager->getEventsFile("global.txt").string_view());

    std::vector<std::string_view> lines = split(txt, '\n');
    for (std::string_view &line : lines)
        if (line.ends_with('\r'))
            line = line.substr(0, line.size() - 1);

    std::vector<std::string_view> chunks;
    for (std::string_view line : std::views::drop(lines, 1)) {
        if (line.empty())
            continue;

        split(line, '\t', &chunks);
        if (chunks.size() != 2)
            throw Exception("Invalid localization file");

        int id = fromString<int>(chunks[0]);
        std::string text = trimRemoveQuotes(chunks[1]);

        std::string enumName = toUpperCaseEnum(text);
        if (enumName.size() > 40) {
            auto pos = enumName.rfind('_', 40);
            if (pos != std::string_view::npos)
                enumName = enumName.substr(0, pos);
        }

        if (id == 72) {
            enumName = "EMPTY_SAVE";
        } else if (id == 79) {
            enumName = "EXIT_DIALOGUE";
        } else if (id == 99 || id == 101 || id == 103 || id == 106) {
            enumName = "RACE_" + enumName;
        } else if (id == 379 || id == 392 || id == 399 || id == 402 || id == 434) {
            enumName = "REPUTATION_" + enumName;
        } else if (id >= 506 && id <= 509) {
            enumName = "NAME_" + enumName;
        } else if (id >= 578 && id <= 581) {
            enumName = "ARENA_DIFFICULTY_" + enumName;
        } else if (id == 630) {
            enumName = "UNKNOWN_VALUE";
        } else if (id == 675) {
            enumName = "GOOD_ENDING";
        } else if (id == 676) {
            enumName = "EVIL_ENDING";
        } else if (enumName == "DAY" || enumName == "SIR" || enumName == "LADY") {
            enumName += std::isupper(text[0]) ? "_CAPITALIZED" : "_LOWERCASE";
        }

        if (text.size() > 80) {
            auto pos = text.find_last_of(",. ", 80);
            if (pos != std::string_view::npos)
                text = text.substr(0, pos) + "...";
        }

        for (char &c : text)
            if (c < '\0' || c > '\x7f')
                c = ' ';

        map.insert(id, enumName, "\"" + text + "\"");
    }

    map.dump(stdout, "LSTR_");
    return 0;
}

int platformMain(int argc, char **argv) {
    try {
        UnicodeCrt _(argc, argv);
        CodeGenOptions options = CodeGenOptions::parse(argc, argv);
        if (options.helpPrinted)
            return 1;

        GameStarter starter(options);

        GameResourceManager resourceManager;
        resourceManager.openGameResources();

        switch (options.subcommand) {
        case CodeGenOptions::SUBCOMMAND_ITEM_ID: return runItemIdCodeGen(options, &resourceManager);
        case CodeGenOptions::SUBCOMMAND_MAP_ID: return runMapIdCodeGen(options, &resourceManager);
        case CodeGenOptions::SUBCOMMAND_BEACON_MAPPING: return runBeaconsCodeGen(options, &resourceManager);
        case CodeGenOptions::SUBCOMMAND_HOUSE_ID: return runHouseIdCodeGen(options, &resourceManager);
        case CodeGenOptions::SUBCOMMAND_MONSTER_ID: return runMonsterIdCodeGen(options, &resourceManager);
        case CodeGenOptions::SUBCOMMAND_MONSTER_TYPE: return runMonsterTypeCodeGen(options, &resourceManager);
        case CodeGenOptions::SUBCOMMAND_BOUNTY_HUNT: return runBountyHuntCodeGen(options, &resourceManager);
        case CodeGenOptions::SUBCOMMAND_MUSIC: return runMusicCodeGen(options, &resourceManager);
        case CodeGenOptions::SUBCOMMAND_DECORATIONS: return runDecorationsCodegen(options, &resourceManager);
        case CodeGenOptions::SUBCOMMAND_SPEECH_PORTRAITS: return runSpeechPortraitsCodegen(options, &resourceManager);
        case CodeGenOptions::SUBCOMMAND_LSTR_ID: return runLstrCodegen(options, &resourceManager);
        default:
            assert(false);
            return 1;
        }
    } catch (const std::exception &e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
}
