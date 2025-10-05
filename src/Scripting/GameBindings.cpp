#include "GameBindings.h"

#include <algorithm>
#include <string>
#include <utility>
#include <string_view>
#include <memory>
#include <vector>
#include <ranges>
#include <optional>
#include <sol/sol.hpp>

#include "Engine/Party.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Random/Random.h"
#include "Engine/mm7_data.h"
#include "Library/Logger/Logger.h"
#include "Media/Audio/AudioPlayer.h"
#include "Media/Audio/SoundList.h"

#include "GUI/GUIWindow.h"

#include "Utility/Exception.h"

Character *getCharacterByIndex(int characterIndex);
sol::table createCharacterConditionTable(sol::state_view &luaState, const Character &character);
sol::table createCharacterSkillsTable(sol::state_view &luaState, const Character &character);
std::unique_ptr<LuaItemQueryTable<Character>> GameBindings::_characterInfoQueryTable;

GameBindings::GameBindings() = default;
GameBindings::~GameBindings() {
    // TODO(Gerark) This static variable must be moved elsewhere
    _characterInfoQueryTable = nullptr;
}

sol::table GameBindings::createBindingTable(sol::state_view &solState) const {
    /** TODO(Gerark) exposing the info/stats of a character this way might suggest we should expose the Character class directly to lua.
    *   The idea is to wait till we'll talk about serious modding/scripting and not taking a direction upfront
    *   TODO(Gerark) This check is also a hack to avoid initializing the static characterInfoTable. multiple times,
    *   Needs to be moved to another place where the ScriptingSystem is providing binding helpers */
    if (!_characterInfoQueryTable) {
        _characterInfoQueryTable = std::make_unique<LuaItemQueryTable<Character>>(solState);
        _characterInfoQueryTable->add("name", [](auto &character) { return character.name; });
        _characterInfoQueryTable->add("xp", [](auto &character) { return character.experience; });
        _characterInfoQueryTable->add("sp", [](auto &character) { return character.uSkillPoints; });
        _characterInfoQueryTable->add("mana", [](auto &character) { return character.GetMana(); });
        _characterInfoQueryTable->add("maxMana", [](auto &character) { return character.GetMaxMana(); });
        _characterInfoQueryTable->add("hp", [](auto &character) { return character.GetHealth(); });
        _characterInfoQueryTable->add("maxHp", [](auto &character) { return character.GetMaxHealth(); });
        _characterInfoQueryTable->add("condition", [&solState](auto &character) { return createCharacterConditionTable(solState, character); });
        _characterInfoQueryTable->add("skills", [&solState](auto &character) { return createCharacterSkillsTable(solState, character); });
        _characterInfoQueryTable->add("class", [this](auto &character) { return character.classType; });
    }

    sol::table table = solState.create_table();
    _registerMiscBindings(solState, table);
    _registerPartyBindings(solState, table);
    _registerItemBindings(solState, table);
    _registerEnums(solState, table);
    _registerFunctions(solState, table);
    return table;
}

void GameBindings::_registerMiscBindings(sol::state_view &solState, sol::table &table) const {
    //TODO(Gerark) We shouldn't have a misc table but it will disappear soon
    table["misc"] = solState.create_table_with(
        "goToScreen", sol::as_function([](int screenIndex) {
        SetCurrentMenuID(MenuType(screenIndex));
    }),
        "canClassLearn", sol::as_function([](Class classType, Skill skillType) {
        return skillMaxMasteryPerClass[classType][skillType] > MASTERY_NONE;
    })
    );
}

void GameBindings::_registerPartyBindings(sol::state_view &solState, sol::table &table) const {
    table["party"] = solState.create_table_with(
        "getGold", sol::as_function([]() {
            return pParty->GetGold();
        }),
        "setGold", sol::as_function([](int amount) {
            pParty->SetGold(amount);
        }),
        "getFood", sol::as_function([]() {
            return pParty->GetFood();
        }),
        "setFood", sol::as_function([](int food) {
            pParty->SetFood(food);
        }),
        "getAlignment", sol::as_function([]() {
            return pParty->alignment;
        }),
        "setAlignment", sol::as_function([](PartyAlignment alignment) {
            pParty->alignment = alignment;
            SetUserInterface(pParty->alignment);
        }),
        "givePartyXp", sol::as_function([](int amount) {
            pParty->GivePartyExp(amount);
        }),
        "getPartySize", sol::as_function([]() {
            return pParty->pCharacters.size();
        }),
        "getActiveCharacter", sol::as_function([]() {
            if (pParty->hasActiveCharacter()) {
                int index = pParty->activeCharacterIndex();
                assert(index != 0); //keep an assert here in case we change the 1-based index to 0 in the future so we can adjust it accordingly
                return index; //a 1-based index is totally fine for lua
            } else {
                return 0;
            }
        }),
        "getCharacterInfo", sol::as_function([this, &solState](int characterIndex, QueryTable queryTable) {
            if (Character *character = getCharacterByIndex(characterIndex - 1)) {
                return _characterInfoQueryTable->createTable(*character, queryTable);
            }
            return sol::make_object(solState, sol::lua_nil);
        }),
        "setCharacterInfo", sol::as_function([](int characterIndex, const sol::object &info) {
            if (Character *character = getCharacterByIndex(characterIndex - 1)) {
                const sol::table &table = info.as<sol::table>();
                for (auto &&val : table) {
                    std::string_view key = val.first.as<std::string_view>();
                    if (key == "xp") {
                        character->experience = val.second.as<int>();
                    } else if (key == "sp") {
                        character->uSkillPoints = val.second.as<int>();
                    } else if (key == "hp") {
                        character->health = val.second.as<int>();
                    } else if (key == "mana") {
                        character->mana = val.second.as<int>();
                    } else if (key == "class") {
                        character->classType = val.second.as<Class>();
                    } else if (key == "condition") {
                        character->SetCondition(val.second.as<Condition>(), false);
                    } else if (key == "skill") {
                        sol::table skillValueTable = val.second.as<sol::table>();
                        CombinedSkillValue current = character->getActualSkillValue(skillValueTable["id"]);

                        auto level = skillValueTable.get<std::optional<int>>("level");
                        if (!level)
                            level = current.level();

                        auto mastery = skillValueTable.get<std::optional<Mastery>>("mastery");
                        if (!mastery)
                            mastery = current.mastery();

                        if (!CombinedSkillValue::isValid(*level, *mastery))
                            throw Exception("Invalid skill-mastery pair '{} {}'", *level, static_cast<int>(*mastery)); // TODO(captainurist): #enum need proper toDisplayString.

                        character->setSkillValue(skillValueTable["id"], CombinedSkillValue(*level, *mastery));
                    } else {
                        throw Exception("Invalid key for set_character_info. Used key: {}", key);
                    }
                }
            }
        }),
        "addItemToInventory", sol::as_function([](int characterIndex, ItemId itemId) {
            if (Character *character = getCharacterByIndex(characterIndex - 1))
                return !!character->inventory.tryAdd(Item(itemId));
            return false;
        }),
        "addCustomItemToInventory", sol::as_function([](int characterIndex, sol::table itemTable) {
            if (Character *character = getCharacterByIndex(characterIndex - 1)) {
                Item item;
                for (auto &&pair : itemTable) {
                    std::string_view key = pair.first.as<std::string_view>();
                    if (key == "id") {
                        item.itemId = pair.second.as<ItemId>();
                    } else if (key == "holder") {
                        item.lichJarCharacterIndex = pair.second.as<int>() - 1; // character index in lua is 1-based
                    }
                }
                return !!character->inventory.tryAdd(item);
            }
            return false;
        }),
        "playAllCharactersAwardSound", sol::as_function([]() {
            for (auto &&character : pParty->pCharacters) {
                character.PlayAwardSound_Anim();
            }
        }),
        "playCharacterAwardSound", sol::as_function([](int characterIndex) {
            if (Character *character = getCharacterByIndex(characterIndex - 1)) {
                character->PlayAwardSound_Anim();
            }
        }),
        "clearCondition", sol::as_function([](int characterIndex, std::optional<Condition> conditionToClear) {
            if (Character *character = getCharacterByIndex(characterIndex - 1)) {
                if (conditionToClear) {
                    character->conditions.reset(*conditionToClear);
                } else {
                    character->conditions.resetAll();
                }
            }
        }),
        "getQBit", sol::as_function([](QuestBit qbit) {
            return pParty->_questBits.test(qbit);
        }),
        "setQBit", sol::as_function([](QuestBit qbit, bool value) {
            pParty->_questBits.set(qbit, value);
        })
    );
}

void GameBindings::_registerItemBindings(sol::state_view &solState, sol::table &table) const {
    using FilterItemFunction = std::function<bool(ItemId)>;

    auto createItemTable = [&solState](const ItemData &itemDesc) {
        return solState.create_table_with(
            "name", itemDesc.name,
            "level", itemDesc.identifyAndRepairDifficulty
        );
    };

    table["items"] = solState.create_table_with(
        "getItemInfo", sol::as_function([&solState, createItemTable](ItemId itemId) {
            if (itemId >= ITEM_FIRST_VALID && itemId <= ITEM_LAST_VALID) {
                const ItemData &itemDesc = pItemTable->items[itemId];
                return sol::object(solState, createItemTable(itemDesc));
            }
            return sol::make_object(solState, sol::lua_nil);
        }),
        // The getRandomItem function accept an optional filter function to exclude some items from the randomization
        "getRandomItem", sol::as_function([](const FilterItemFunction &filter) {
            if (filter) {
                std::vector<ItemId> itemsToRandomizeOn;
                Segment<ItemId> &&spawnableItems = allSpawnableItems();
                for (ItemId itemId : spawnableItems | std::views::filter(filter)) {
                    itemsToRandomizeOn.push_back(itemId);
                }
                return grng->randomSample(itemsToRandomizeOn);
            }
            return grng->randomSample(allSpawnableItems());
        })
    );
}

void GameBindings::_registerEnums(sol::state_view &solState, sol::table &table) const {
    //TODO(captainurist): Use serialization tables to automate this.
    table.new_enum<false>("PartyAlignment",
        "Good", PartyAlignment::PartyAlignment_Good,
        "Neutral", PartyAlignment::PartyAlignment_Neutral,
        "Evil", PartyAlignment::PartyAlignment_Evil
    );

    table.new_enum<false>("CharacterCondition",
        "Cursed", CONDITION_CURSED,
        "Weak", CONDITION_WEAK,
        "Sleep", CONDITION_SLEEP,
        "Fear", CONDITION_FEAR,
        "Drunk", CONDITION_DRUNK,
        "Insane", CONDITION_INSANE,
        "Poison_weak", CONDITION_POISON_WEAK,
        "Disease_weak", CONDITION_DISEASE_WEAK,
        "Poison_medium", CONDITION_POISON_MEDIUM,
        "Disease_medium", CONDITION_DISEASE_MEDIUM,
        "Poison_severe", CONDITION_POISON_SEVERE,
        "Disease_severe", CONDITION_DISEASE_SEVERE,
        "Paralyzed", CONDITION_PARALYZED,
        "Unconscious", CONDITION_UNCONSCIOUS,
        "Dead", CONDITION_DEAD,
        "Petriefied", CONDITION_PETRIFIED,
        "Eradicated", CONDITION_ERADICATED,
        "Zombie", CONDITION_ZOMBIE,
        "Good", CONDITION_GOOD
    );

    table.new_enum<false>("SkillType",
        "Staff", SKILL_STAFF,
        "Sword", SKILL_SWORD,
        "Dagger", SKILL_DAGGER,
        "Axe", SKILL_AXE,
        "Spear", SKILL_SPEAR,
        "Bow", SKILL_BOW,
        "Mace", SKILL_MACE,
        "Blaster", SKILL_BLASTER,
        "Shield", SKILL_SHIELD,
        "Leather", SKILL_LEATHER,
        "Chain", SKILL_CHAIN,
        "Plate", SKILL_PLATE,
        "Fire", SKILL_FIRE,
        "Air", SKILL_AIR,
        "Water", SKILL_WATER,
        "Earth", SKILL_EARTH,
        "Spirit", SKILL_SPIRIT,
        "Mind", SKILL_MIND,
        "Body", SKILL_BODY,
        "Light", SKILL_LIGHT,
        "Dark", SKILL_DARK,
        "Item_ID", SKILL_ITEM_ID,
        "Merchant", SKILL_MERCHANT,
        "Repair", SKILL_REPAIR,
        "Bodybuilding", SKILL_BODYBUILDING,
        "Meditation", SKILL_MEDITATION,
        "Perception", SKILL_PERCEPTION,
        "Diplomacy", SKILL_DIPLOMACY,
        "Thievery", SKILL_THIEVERY,
        "Trap_Disarm", SKILL_TRAP_DISARM,
        "Dodge", SKILL_DODGE,
        "Unarmed", SKILL_UNARMED,
        "Monster_ID", SKILL_MONSTER_ID,
        "Armsmaster", SKILL_ARMSMASTER,
        "Stealing", SKILL_STEALING,
        "Alchemy", SKILL_ALCHEMY,
        "Learning", SKILL_LEARNING,
        "Club", SKILL_CLUB,
        "Misc", SKILL_MISC
    );

    table.new_enum<false>("SkillMastery",
        "None", MASTERY_NONE,
        "Novice", MASTERY_NOVICE,
        "Expert", MASTERY_EXPERT,
        "Master", MASTERY_MASTER,
        "Grandmaster", MASTERY_GRANDMASTER
    );

    table.new_enum<false>("ClassType",
        "Knight", CLASS_KNIGHT,
        "Cavalier", CLASS_CAVALIER,
        "Champion", CLASS_CHAMPION,
        "BlackKnight", CLASS_BLACK_KNIGHT,
        "Thief", CLASS_THIEF,
        "Rogue", CLASS_ROGUE,
        "Spy", CLASS_SPY,
        "Assassin", CLASS_ASSASSIN,
        "Monk", CLASS_MONK,
        "Initiate", CLASS_INITIATE,
        "Master", CLASS_MASTER,
        "Ninja", CLASS_NINJA,
        "Paladin", CLASS_PALADIN,
        "Crusader", CLASS_CRUSADER,
        "Hero", CLASS_HERO,
        "Villain", CLASS_VILLIAN,
        "Archer", CLASS_ARCHER,
        "WarriorMage", CLASS_WARRIOR_MAGE,
        "MasterArcher", CLASS_MASTER_ARCHER,
        "Sniper", CLASS_SNIPER,
        "Ranger", CLASS_RANGER,
        "Hunter", CLASS_HUNTER,
        "RangerLord", CLASS_RANGER_LORD,
        "BountyHunter", CLASS_BOUNTY_HUNTER,
        "Cleric", CLASS_CLERIC,
        "Priest", CLASS_PRIEST,
        "PriestOfSun", CLASS_PRIEST_OF_SUN,
        "PriestOfMoon", CLASS_PRIEST_OF_MOON,
        "Druid", CLASS_DRUID,
        "GreatDruid", CLASS_GREAT_DRUID,
        "ArchDruid", CLASS_ARCH_DRUID,
        "Warlock", CLASS_WARLOCK,
        "Sorcerer", CLASS_SORCERER,
        "Wizard", CLASS_WIZARD,
        "Archmage", CLASS_ARCHAMGE,
        "Lich", CLASS_LICH
    );

    table.new_enum<false>("QBits",
        "DarkPath", QBIT_DARK_PATH,
        "LightPath", QBIT_LIGHT_PATH
    );

    sol::table itemTypeEnum = solState.create_table();
    std::vector<std::pair<std::string, ItemId>> sortedItems;

    for (ItemId itemId : pItemTable->items.indices()) {
        const ItemData& itemDesc = pItemTable->items[itemId];
        if (!itemDesc.name.empty()) {
            std::string luaKey = itemDesc.name;
            std::replace(luaKey.begin(), luaKey.end(), ' ', '_');
            sortedItems.emplace_back(luaKey, itemId);
        }
    }

    // Sort alphabetically by name
    std::sort(sortedItems.begin(), sortedItems.end());

    for (const auto& [name, id] : sortedItems) {
        itemTypeEnum[name] = std::to_underlying(id);
    }

    table["ItemType"] = itemTypeEnum;
}

void GameBindings::_registerFunctions(sol::state_view &solState, sol::table &table) const {
    table["debugCallback"] = sol::as_function([] {
        // Do nothing. You can write your code here.
    });
}

Character *getCharacterByIndex(int characterIndex) {
    if (characterIndex >= 0 && characterIndex < pParty->pCharacters.size()) {
        return &pParty->pCharacters[characterIndex];
    }

    logger->warning("Invalid character index. Asked for: {} but the party size is: {}", characterIndex, pParty->pCharacters.size());
    return nullptr;
}

sol::table createCharacterConditionTable(sol::state_view &luaState, const Character &character) {
    sol::table result = luaState.create_table();

    for (auto &&condition : allConditions()) {
        if (character.conditions.has(condition)) {
            result[condition] = true;
        }
    }
    return result;
}

sol::table createCharacterSkillsTable(sol::state_view &luaState, const Character &character) {
    sol::table result = luaState.create_table();
    for (Skill skillType : character.pActiveSkills.indices()) {
        if (character.HasSkill(skillType)) {
            CombinedSkillValue skillValue = character.getActualSkillValue(skillType);
            result[skillType] = luaState.create_table_with(
                "level", skillValue.level(),
                "mastery", skillValue.mastery()
            );
        }
    }
    return result;
}
