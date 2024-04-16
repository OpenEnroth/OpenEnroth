#include "GameLuaBindings.h"

#include <string_view>
#include <memory>
#include <vector>
#include <sol/sol.hpp>

#include "Engine/Party.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Random/Random.h"
#include "Library/Logger/Logger.h"
#include "Media/Audio/AudioPlayer.h"
#include "Media/Audio/SoundList.h"

#include "GUI/GUIWindow.h"

void _registerAudioBindings(sol::state_view &luaState, sol::table &table);
void _registerGameBindings(sol::state_view &luaState, sol::table &table);
void _registerItemBindings(sol::state_view &luaState, sol::table &table);
void _registerSerializationBindings(sol::state_view &luaState, sol::table &table);

Character *getCharacterByIndex(int characterIndex);

GameLuaBindings::GameLuaBindings() = default;
GameLuaBindings::~GameLuaBindings() = default;

void GameLuaBindings::init(lua_State *L) {
    _luaState = std::make_unique<sol::state_view>(L);

    _luaState->set_function("initMMBindings", [luaState = _luaState.get()]() {
        sol::table mainTable = luaState->create_table();
        _registerGameBindings(*luaState, mainTable);
        _registerItemBindings(*luaState, mainTable);
        _registerAudioBindings(*luaState, mainTable);
        _registerSerializationBindings(*luaState, mainTable);
        return mainTable;
    });
}

void _registerGameBindings(sol::state_view &luaState, sol::table& table) {
    //TODO(captainurist): Use serialization tables to automate this.
    luaState.new_enum("PartyAlignment",
        "Good", PartyAlignment::PartyAlignment_Good,
        "Neutral", PartyAlignment::PartyAlignment_Neutral,
        "Evil", PartyAlignment::PartyAlignment_Evil
    );

    table["game"] = luaState.create_table_with(
        "get_gold", []() {
            return pParty->GetGold();
        },
        "set_gold", [](int amount) {
            pParty->SetGold(amount);
        },
        "get_food", []() {
            return pParty->GetFood();
        },
        "set_food", [](int food) {
            pParty->SetFood(food);
        },
        "get_alignment", []() {
            return pParty->alignment;
        },
        "set_alignment", [](PartyAlignment alignment) {
            pParty->alignment = alignment;
            SetUserInterface(pParty->alignment);
        },
        "give_party_xp", [](int amount) {
            pParty->GivePartyExp(amount);
        },
        "get_party_size", []() {
            return pParty->pCharacters.size();
        },
        "get_active_character", []() {
            if(pParty->hasActiveCharacter()) {
                int index = pParty->activeCharacterIndex();
                assert(index != 0); //keep an assert here in case we change the 1-based index to 0 in the future so we can adjust it accordingly
                return index; //a 1-based index is totally fine for lua
            } else {
                return 0;
            }
        },
        "get_character_info", [&luaState](int characterIndex) {
            if(Character *character = getCharacterByIndex(characterIndex - 1); character != nullptr) {
                return sol::make_object(luaState, luaState.create_table_with(
                    "name", character->name,
                    "xp", character->experience,
                    "sp", character->uSkillPoints
                ));
            }
            return sol::make_object(luaState, sol::nil);
        },
        "set_character_info", [](int characterIndex, const sol::object &info) {
            if(Character *character = getCharacterByIndex(characterIndex - 1); character != nullptr) {
                const sol::table &table = info.as<sol::table>();
                for (auto &&val : table) {
                    std::string_view key = val.first.as<std::string_view>();
                    if (key == "xp") {
                        character->experience = val.second.as<int>();
                    } else if(key == "sp") {
                        character->uSkillPoints = val.second.as<int>();
                    } else {
                        logger->warning("Invalid key for set_character_info. Used key: {}", key);
                    }
                }
            }
        },
        "add_item_to_inventory", [](int characterIndex, ItemId itemId) {
            if(Character *character = getCharacterByIndex(characterIndex - 1); character != nullptr) {
                return character->AddItem(-1, itemId) != 0;
            }
            return false;
        },
        "play_all_characters_award_sound", []() {
            for (auto &&character : pParty->pCharacters) {
                character.PlayAwardSound_Anim();
            }
        },
        "play_character_award_sound", [](int characterIndex) {
            if(Character *character = getCharacterByIndex(characterIndex - 1); character != nullptr) {
                character->PlayAwardSound_Anim();
            }
        },
        "go_to_screen", [](int screenIndex) {
            SetCurrentMenuID(MenuType(screenIndex));
        }
    );
}

void _registerAudioBindings(sol::state_view &luaState, sol::table &table) {
    table["audio"] = luaState.create_table_with(
        "playSound", [](SoundId soundId, SoundPlaybackMode mode) {
            pAudioPlayer->playSound(soundId, mode);
        },
        "playMusic", [](MusicId musicId) {
            pAudioPlayer->MusicPlayTrack(musicId);
        }
    );
}

void _registerItemBindings(sol::state_view &luaState, sol::table &table) {
    typedef std::function<bool(ItemId)> FilteItemFunction;

    auto createItemTable = [&luaState](const ItemDesc& itemDesc) {
        return luaState.create_table_with(
            "name", itemDesc.name,
            "level", itemDesc.uItemID_Rep_St
        );
    };

    table["items"] = luaState.create_table_with(
        "get_item_info", [&luaState, createItemTable](ItemId itemId) {
            if(itemId >= ITEM_FIRST_VALID && itemId <= ITEM_LAST_VALID) {
                const ItemDesc &itemDesc = pItemTable->pItems[itemId];
                return sol::object(luaState, createItemTable(itemDesc));
            }
            return sol::make_object(luaState, sol::nil);
        },
        // The get_random_item function accept an optional filter function to exclude some items from the randomization
        "get_random_item", [](const FilteItemFunction& filter) {
            if(filter) {
                std::vector<ItemId> itemsToRandomizeOn;
                Segment<ItemId> &&spawnableItems = allSpawnableItems();
                for (ItemId itemId : spawnableItems) {
                    if (filter(itemId)) {
                        itemsToRandomizeOn.push_back(itemId);
                    }
                }
                return grng->randomSample(itemsToRandomizeOn);
            }
            return grng->randomSample(allSpawnableItems());
        }
    );
}

void _registerSerializationBindings(sol::state_view &luaState, sol::table &table) {
    //Exposing serializations and deserializations functions to lua
    //Useful for converting command line strings to the correct types
    table["deserialize"] = luaState.create_table_with(
        "party_alignment", [](std::string_view alignment) {
            return fromString<PartyAlignment>(alignment);
        }
    );

    table["serialize"] = luaState.create_table_with(
        "party_alignment", [](PartyAlignment alignment) {
            return toString(alignment);
        }
    );
}

Character *getCharacterByIndex(int characterIndex) {
    if (characterIndex >= 0 && characterIndex < pParty->pCharacters.size()) {
        return &pParty->pCharacters[characterIndex];
    }

    logger->warning("Invalid character index. Asked for: {} but the party size is: {}", characterIndex, pParty->pCharacters.size());
    return nullptr;
}
