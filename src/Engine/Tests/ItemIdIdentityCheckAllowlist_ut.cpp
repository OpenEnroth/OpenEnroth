#include <filesystem>
#include <fstream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Testing/Unit/UnitTest.h"

// Guards against MM7 item-id identity checks (`itemId == ITEM_X`, `itemId != ITEM_X`, `case ITEM_X:`)
// whose id names a different item in MM8's items.txt.
//
// This is a pure source-text scan. The scan covers `==`, `!=` and case labels in every file under
// `src/` (excluding `Tests/`), and fails if a site shows up that isn't in the classified allowlist
// below. Needs no game data.

namespace {

std::filesystem::path repoRoot() {
#ifdef MM8_REPO_ROOT_DIR
    return std::filesystem::path(MM8_REPO_ROOT_DIR);
#else
    throw std::runtime_error("MM8_REPO_ROOT_DIR not compiled in - can't resolve the repo root.");
#endif
}

std::string readFile(const std::filesystem::path &path) {
    std::ifstream in(path, std::ios::binary);
    if (!in)
        throw std::runtime_error("Couldn't open " + path.string());
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

/**
 * Real `ItemId` enum member names, extracted from `ItemEnums.h` itself (the ground truth). This is
 * how a genuine `ItemId itemId == ITEM_ARTIFACT_X` comparison is told apart from an unrelated
 * `ItemType`/`ItemEnchantment`/`ItemSlot`/`ItemSource`/`ItemTreasureLevel`/`ItemFlag` constant that
 * also happens to start with `ITEM_` (e.g. `ITEM_TYPE_ARMOUR`, `ITEM_ENCHANTMENT_NULL`,
 * `ITEM_SLOT_BELT`) - those are members of entirely different enums.
 */
std::set<std::string> itemIdMemberNames() {
    std::string text = readFile(repoRoot() / "src/Engine/Objects/ItemEnums.h");

    size_t begin = text.find("enum class ItemId");
    if (begin == std::string::npos)
        throw std::runtime_error("Couldn't find 'enum class ItemId' in ItemEnums.h - has it been renamed?");
    size_t end = text.find("};", begin);
    if (end == std::string::npos)
        throw std::runtime_error("Couldn't find the closing '};' for enum class ItemId.");
    std::string body = text.substr(begin, end - begin);

    static const std::regex memberPattern(R"(\b(ITEM_[A-Za-z0-9_]+)\s*=)");
    std::set<std::string> names;
    for (auto it = std::sregex_iterator(body.begin(), body.end(), memberPattern); it != std::sregex_iterator(); ++it)
        names.insert((*it)[1].str());
    return names;
}

/**
 * All `.cpp`/`.h` files under `src/`, relative to the repo root, excluding anything under a
 * `Tests/` directory (unit tests routinely construct arbitrary `ItemId` values for fixtures, which
 * isn't the "hardcoded named constant in production logic" bug class this guard targets).
 */
std::vector<std::string> sourceFilesUnderSrc() {
    std::vector<std::string> result;
    std::filesystem::path root = repoRoot() / "src";
    for (const std::filesystem::directory_entry &entry : std::filesystem::recursive_directory_iterator(root)) {
        if (!entry.is_regular_file())
            continue;
        const std::filesystem::path &path = entry.path();
        if (path.extension() != ".cpp" && path.extension() != ".h")
            continue;
        std::string relative = std::filesystem::relative(path, repoRoot()).generic_string();
        if (relative.find("/Tests/") != std::string::npos)
            continue;
        result.push_back(relative);
    }
    return result;
}

/** (file, constant) -> number of `==`/`!=`/`case` identity occurrences of that constant in that file. */
std::map<std::pair<std::string, std::string>, int> findItemIdIdentityCheckSites(const std::vector<std::string> &relativeFiles) {
    std::set<std::string> itemIdMembers = itemIdMemberNames();
    static const std::regex comparisonPattern(R"([!=]=\s*(ITEM_[A-Za-z0-9_]+))");
    static const std::regex casePattern(R"(\bcase\s+(ITEM_[A-Za-z0-9_]+)\s*:)");

    std::map<std::pair<std::string, std::string>, int> counts;
    for (const std::string &relativeFile : relativeFiles) {
        std::string text = readFile(repoRoot() / relativeFile);
        for (const std::regex &pattern : {comparisonPattern, casePattern}) {
            for (auto it = std::sregex_iterator(text.begin(), text.end(), pattern); it != std::sregex_iterator(); ++it) {
                std::string constant = (*it)[1].str();
                if (itemIdMembers.contains(constant))
                    counts[{relativeFile, constant}]++;
            }
        }
    }
    return counts;
}

enum class ItemIdSiteClassification {
    DEAD_FOR_MM8,    // Unreachable in MM8 (structurally, the real id is an unspawnable data placeholder, or the real id's item type/slot can never reach this comparison).
    // Reachable in MM8, real item(s) at this id (or id range) match the code's assumption (or it's a sentinel
    // unaffected by the block-shift, e.g. ITEM_NULL; or an id-range-keyed mechanic that doesn't depend on the
    // constant's MM7 flavor name, e.g. potions/spellbooks/reagents).
    LIVE_CORRECT,
    LIVE_WRONG_FIXED // Wrong in MM8 unless gated (real id names an unrelated item), so gated to gameType() == GAME_TYPE_MM7.
};

struct AllowlistEntry {
    const char *file;
    const char *constant;
    int count;
    ItemIdSiteClassification classification;
};

const AllowlistEntry kAllowlist[] = {
    {"src/Application/Game.cpp", "ITEM_NULL", 1, ItemIdSiteClassification::LIVE_CORRECT},

    // `itemTarget != ITEM_NULL` guards whether `castSpell`'s caller passed a real
    // inventory-item target for an `ON_CAST_TargetedEnchantment` spell - a sentinel check, not a
    // game-content identity check, so unaffected by the MM7/MM8 item-id block-shift.
    {"src/Engine/Components/Control/EngineController.cpp", "ITEM_NULL", 1, ItemIdSiteClassification::LIVE_CORRECT},

    {"src/Engine/Components/Trace/EngineTraceStateAccessor.cpp", "ITEM_POTION_BOTTLE", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Components/Trace/EngineTraceStateAccessor.cpp", "ITEM_QUEST_LICH_JAR_FULL", 1, ItemIdSiteClassification::DEAD_FOR_MM8},

    {"src/Engine/Conditions.cpp", "ITEM_NULL", 1, ItemIdSiteClassification::LIVE_CORRECT},

    {"src/Engine/Engine.cpp", "ITEM_ARTIFACT_HERMES_SANDALS", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Engine.cpp", "ITEM_ARTIFACT_HEROS_BELT", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Engine.cpp", "ITEM_ARTIFACT_MINDS_EYE", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Engine.cpp", "ITEM_NULL", 4, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Engine.cpp", "ITEM_RELIC_ETHRICS_STAFF", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},

    {"src/Engine/Evt/EvtInterpreter.cpp", "ITEM_NULL", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Interaction.cpp", "ITEM_ARTIFACT_SPLITTER", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Interaction.cpp", "ITEM_NULL", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Interaction.cpp", "ITEM_SPELLBOOK_REMOVE_FEAR", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},

    {"src/Engine/Objects/Actor.cpp", "ITEM_NULL", 12, ItemIdSiteClassification::LIVE_CORRECT},

    {"src/Engine/Objects/Character.cpp", "ITEM_161", 2, ItemIdSiteClassification::DEAD_FOR_MM8},
    {"src/Engine/Objects/Character.cpp", "ITEM_162", 1, ItemIdSiteClassification::DEAD_FOR_MM8},
    {"src/Engine/Objects/Character.cpp", "ITEM_ARTIFACT_ELFBANE", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/Character.cpp", "ITEM_ARTIFACT_ELVEN_CHAINMAIL", 2, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/Character.cpp", "ITEM_ARTIFACT_FORGE_GAUNTLETS", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/Character.cpp", "ITEM_ARTIFACT_GHOULSBANE", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/Character.cpp", "ITEM_ARTIFACT_GIBBET", 3, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/Character.cpp", "ITEM_ARTIFACT_HEROS_BELT", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/Character.cpp", "ITEM_ARTIFACT_LADYS_ESCORT", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/Character.cpp", "ITEM_ARTIFACT_MINDS_EYE", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/Character.cpp", "ITEM_ARTIFACT_PUCK", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/Character.cpp", "ITEM_GENIE_LAMP", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/Character.cpp", "ITEM_HORSESHOE", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/Character.cpp", "ITEM_NULL", 12, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_ACCURACY_BOOST", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_AIR_RESISTANCE", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_AWAKEN", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_BLESS", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_BODY_RESISTANCE", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_CATALYST", 2, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_CURE_DISEASE", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_CURE_INSANITY", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_CURE_PARALYSIS", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_CURE_POISON", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_CURE_WEAKNESS", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_CURE_WOUNDS", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_DIVINE_CURE", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_DIVINE_POWER", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_DIVINE_RESTORATION", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_EARTH_RESISTANCE", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_ENDURANCE_BOOST", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_FIRE_RESISTANCE", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_HASTE", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_HEROISM", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_INTELLECT_BOOST", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_LUCK_BOOST", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_MAGIC", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_MIGHT_BOOST", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_MIND_RESISTANCE", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_PERSONALITY_BOOST", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_PRESERVATION", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_PURE_ACCURACY", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_PURE_ENDURANCE", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_PURE_INTELLECT", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_PURE_LUCK", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_PURE_MIGHT", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_PURE_PERSONALITY", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_PURE_SPEED", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_REJUVENATION", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_REMOVE_CURSE", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_REMOVE_FEAR", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_SHIELD", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_SPEED_BOOST", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_STONESKIN", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_STONE_TO_FLESH", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_WATER_BREATHING", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_POTION_WATER_RESISTANCE", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_QUEST_FAERIE_PIPES", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/Character.cpp", "ITEM_QUEST_GRYPHONHEARTS_TRUMPET", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Evt/EvtVariables.cpp", "ITEM_QUEST_LICH_JAR_EMPTY", 1, ItemIdSiteClassification::DEAD_FOR_MM8},
    {"src/Engine/Objects/Character.cpp", "ITEM_QUEST_LUTE", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/Character.cpp", "ITEM_QUEST_WETSUIT", 3, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/Character.cpp", "ITEM_RED_APPLE", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/Character.cpp", "ITEM_RELIC_ETHRICS_STAFF", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/Character.cpp", "ITEM_RELIC_JUSTICE", 2, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/Character.cpp", "ITEM_RELIC_OLD_NICK", 2, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/Character.cpp", "ITEM_RELIC_TALEDONS_HELM", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/Character.cpp", "ITEM_RELIC_TWILIGHT", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Evt/EvtVariables.cpp", "ITEM_SPELLBOOK_CURE_POISON", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Evt/EvtVariables.cpp", "ITEM_SPELLBOOK_LIGHT_BOLT", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Evt/EvtVariables.cpp", "ITEM_SPELLBOOK_REGENERATION", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Character.cpp", "ITEM_TEMPLE_IN_A_BOTTLE", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},

    {"src/Engine/Objects/Chest.cpp", "ITEM_NULL", 3, ItemIdSiteClassification::LIVE_CORRECT},

    {"src/Engine/Objects/Inventory.cpp", "ITEM_NULL", 12, ItemIdSiteClassification::LIVE_CORRECT},

    {"src/Engine/Objects/Inventory.h", "ITEM_NULL", 2, ItemIdSiteClassification::LIVE_CORRECT},

    {"src/Engine/Objects/Item.cpp", "ITEM_ARTIFACT_GHOULSBANE", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/Item.cpp", "ITEM_ARTIFACT_IRON_FEATHER", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/Item.cpp", "ITEM_ARTIFACT_ULLYSES", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/Item.cpp", "ITEM_NULL", 5, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Item.cpp", "ITEM_POTION_BOTTLE", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/Item.cpp", "ITEM_QUEST_LICH_JAR_FULL", 1, ItemIdSiteClassification::DEAD_FOR_MM8},
    {"src/Engine/Objects/Item.cpp", "ITEM_RELIC_OLD_NICK", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},

    {"src/Engine/Objects/ItemEnumFunctions.h", "ITEM_BLASTER", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/ItemEnumFunctions.h", "ITEM_BLASTER_RIFLE", 1, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/Engine/Objects/ItemEnumFunctions.h", "ITEM_POTION_FLAMING", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/ItemEnumFunctions.h", "ITEM_POTION_FREEZING", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/ItemEnumFunctions.h", "ITEM_POTION_NOXIOUS", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/ItemEnumFunctions.h", "ITEM_POTION_PURE_ACCURACY", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/ItemEnumFunctions.h", "ITEM_POTION_PURE_ENDURANCE", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/ItemEnumFunctions.h", "ITEM_POTION_PURE_INTELLECT", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/ItemEnumFunctions.h", "ITEM_POTION_PURE_LUCK", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/ItemEnumFunctions.h", "ITEM_POTION_PURE_MIGHT", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/ItemEnumFunctions.h", "ITEM_POTION_PURE_PERSONALITY", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/ItemEnumFunctions.h", "ITEM_POTION_PURE_SPEED", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/ItemEnumFunctions.h", "ITEM_POTION_SHOCKING", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/ItemEnumFunctions.h", "ITEM_POTION_SLAYING", 2, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Objects/ItemEnumFunctions.h", "ITEM_POTION_SWIFT", 1, ItemIdSiteClassification::LIVE_CORRECT},

    {"src/Engine/Objects/SpriteObject.cpp", "ITEM_NULL", 3, ItemIdSiteClassification::LIVE_CORRECT}, // Upstream's "is there an item to name in the deletion log" sentinel check.
    {"src/Engine/Objects/SpriteObject.cpp", "ITEM_SPELLBOOK_FIREBALL", 1, ItemIdSiteClassification::LIVE_CORRECT},

    {"src/Engine/Party.cpp", "ITEM_NULL", 2, ItemIdSiteClassification::LIVE_CORRECT},

    {"src/Engine/Snapshots/CompositeSnapshots.cpp", "ITEM_NULL", 2, ItemIdSiteClassification::LIVE_CORRECT},

    {"src/Engine/Snapshots/EntitySnapshots.cpp", "ITEM_NULL", 5, ItemIdSiteClassification::LIVE_CORRECT},

    {"src/Engine/Spells/CastSpellInfo.cpp", "ITEM_NULL", 4, ItemIdSiteClassification::LIVE_CORRECT},

    {"src/Engine/Tables/ItemTable.cpp", "ITEM_POTION_BOTTLE", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/Engine/Tables/ItemTable.cpp", "ITEM_SPELLBOOK_DIVINE_INTERVENTION", 1, ItemIdSiteClassification::LIVE_CORRECT},

    // Empty-slot sentinel for a roster row's Equipment cell, 0 under both item numberings.
    {"src/Engine/Tables/RosterTable.cpp", "ITEM_NULL", 1, ItemIdSiteClassification::LIVE_CORRECT},

    {"src/GUI/UI/Houses/MagicGuild.cpp", "ITEM_NULL", 6, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/Houses/MagicGuild.cpp", "ITEM_SPELLBOOK_DIVINE_INTERVENTION", 1, ItemIdSiteClassification::LIVE_CORRECT},

    {"src/GUI/UI/Houses/Shops.cpp", "ITEM_NULL", 14, ItemIdSiteClassification::LIVE_CORRECT},
    // shopCanEverStock() - an alchemist's top row is potion bottles (MM8 item 220, "Potion Bottle"), the id generateShopItems() puts there.
    {"src/GUI/UI/Houses/Shops.cpp", "ITEM_POTION_BOTTLE", 1, ItemIdSiteClassification::LIVE_CORRECT},

    {"src/GUI/UI/ItemGrid.cpp", "ITEM_NULL", 1, ItemIdSiteClassification::LIVE_CORRECT},

    {"src/GUI/UI/NPCTopics.cpp", "ITEM_NULL", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/NPCTopics.cpp", "ITEM_QUEST_LICH_JAR_FULL", 1, ItemIdSiteClassification::DEAD_FOR_MM8},

    {"src/GUI/UI/UICharacter.cpp", "ITEM_ARTIFACT_HERMES_SANDALS", 1, ItemIdSiteClassification::DEAD_FOR_MM8},
    {"src/GUI/UI/UICharacter.cpp", "ITEM_BLASTER", 2, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/GUI/UI/UICharacter.cpp", "ITEM_NULL", 2, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UICharacter.cpp", "ITEM_PHYNAXIAN_HELM", 1, ItemIdSiteClassification::DEAD_FOR_MM8},
    {"src/GUI/UI/UICharacter.cpp", "ITEM_QUEST_WETSUIT", 3, ItemIdSiteClassification::LIVE_WRONG_FIXED},
    {"src/GUI/UI/UICharacter.cpp", "ITEM_SPELLBOOK_FIRE_AURA", 1, ItemIdSiteClassification::DEAD_FOR_MM8},
    {"src/GUI/UI/UICharacter.cpp", "ITEM_SPELLBOOK_JUMP", 1, ItemIdSiteClassification::DEAD_FOR_MM8},
    {"src/GUI/UI/UICharacter.cpp", "ITEM_SPELLBOOK_TORCH_LIGHT", 1, ItemIdSiteClassification::DEAD_FOR_MM8},

    {"src/GUI/UI/UIGame.cpp", "ITEM_NULL", 2, ItemIdSiteClassification::LIVE_CORRECT},

    {"src/GUI/UI/UIPopup.cpp", "ITEM_NULL", 7, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_POTION_BOTTLE", 2, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_POTION_CATALYST", 7, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_POTION_HARDEN_ITEM", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_POTION_RECHARGE_ITEM", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_REAGENT_CRUSHED_ROSE_PETALS", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_REAGENT_DRAGONS_EYE", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_REAGENT_ELVISH_TOADSTOOL", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_REAGENT_FAE_DUST", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_REAGENT_GARNET", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_REAGENT_HARPY_FEATHER", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_REAGENT_MERCURY", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_REAGENT_METEORITE_FRAGMENT", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_REAGENT_MOONSTONE", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_REAGENT_MUSHROOM", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_REAGENT_OBSIDIAN", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_REAGENT_PHILOSOPHERS_STONE", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_REAGENT_PHIRNA_ROOT", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_REAGENT_POPPYSNAPS", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_REAGENT_RUBY", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_REAGENT_SULFUR", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_REAGENT_VIAL_OF_DEVIL_ICHOR", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_REAGENT_VIAL_OF_OOZE_ENDOPLASM", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_REAGENT_VIAL_OF_TROLL_BLOOD", 1, ItemIdSiteClassification::LIVE_CORRECT},
    {"src/GUI/UI/UIPopup.cpp", "ITEM_REAGENT_WIDOWSWEEP_BERRIES", 1, ItemIdSiteClassification::LIVE_CORRECT},

    {"src/Io/Mouse.cpp", "ITEM_NULL", 2, ItemIdSiteClassification::LIVE_CORRECT},
};

} // namespace

UNIT_TEST(ItemIdIdentityCheckAllowlist, NoUnreviewedNamedItemIdSite) {
    std::map<std::pair<std::string, std::string>, int> found = findItemIdIdentityCheckSites(sourceFilesUnderSrc());

    std::map<std::pair<std::string, std::string>, int> allowed;
    for (const AllowlistEntry &entry : kAllowlist)
        allowed[{entry.file, entry.constant}] = entry.count;

    // Direction 1: every site found in source must be a reviewed, classified allowlist entry with
    // a matching occurrence count. A NEW named-ItemId identity check (or a new occurrence of an
    // already-known one) fails here until it's reviewed and added to kAllowlist above.
    for (const auto &[key, count] : found) {
        const auto &[file, constant] = key;
        auto it = allowed.find(key);
        if (it == allowed.end()) {
            ADD_FAILURE() << "Unreviewed named-ItemId identity check: " << constant << " in " << file
                          << " (" << count << " occurrence(s), == / != / case) is not in kAllowlist. Resolve it "
                          << "against real MM8 items.txt data, classify it (dead for MM8 / live+correct / "
                          << "live+wrong), fix if live+wrong, and add it to kAllowlist in this file.";
        } else if (it->second != count) {
            ADD_FAILURE() << "Occurrence count changed for " << constant << " in " << file << ": expected "
                          << it->second << " (kAllowlist), found " << count << ". A new, unreviewed call site "
                          << "for an already-known constant needs the same review as a brand new one.";
        }
    }

    // Direction 2: every allowlist entry must still be backed by a real source site - catches a
    // stale entry left behind after the code it described was deleted/refactored away.
    for (const auto &[key, count] : allowed) {
        const auto &[file, constant] = key;
        auto it = found.find(key);
        EXPECT_TRUE(it != found.end() && it->second == count)
            << "Stale kAllowlist entry: " << constant << " in " << file << " (expected " << count
            << " occurrence(s)) no longer matches source - update or remove this kAllowlist entry.";
    }
}
