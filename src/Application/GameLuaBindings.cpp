#include "GameLuaBindings.h"

#include <string_view>
#include <memory>
#include <sol/sol.hpp>

#include "Engine/Party.h"
#include "Library/Logger/Logger.h"
#include "Media/Audio/AudioPlayer.h"
#include "Media/Audio/SoundList.h"

#include "GUI/GUIWindow.h"

void _registerAudioBindings(sol::state_view &luaState, sol::table &table);
void _registerGameBindings(sol::state_view &luaState, sol::table &table);
void _registerItemBindings(sol::state_view &luaState, sol::table &table);
void _registerSerializationBindings(sol::state_view &luaState, sol::table &table);

GameLuaBindings::GameLuaBindings() = default;
GameLuaBindings::~GameLuaBindings() = default;

void GameLuaBindings::init(lua_State *L) {
    _luaState = std::make_unique<sol::state_view>(L);

    _luaState->set_function("initMMBindings", [luaState = _luaState.get()]() {
        sol::table mainTable = luaState->create_named_table("mm");
        _registerGameBindings(*luaState, mainTable);
        _registerItemBindings(*luaState, mainTable);
        _registerAudioBindings(*luaState, mainTable);
        _registerSerializationBindings(*luaState, mainTable);
        return mainTable;
    });
}

void _registerGameBindings(sol::state_view &luaState, sol::table& table) {
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
            if(Character *character = pParty->getCharacterByIndex(characterIndex - 1); character != nullptr) {
                return luaState.create_table_with(
                    "name", character->name,
                    "xp", character->experience,
                    "sp", character->uSkillPoints
                );
            }
            return luaState.create_table();
        },
        "set_character_info", [](int characterIndex, const sol::object &info) {
            if(Character *character = pParty->getCharacterByIndex(characterIndex - 1); character != nullptr) {
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
            if(Character *character = pParty->getCharacterByIndex(characterIndex - 1); character != nullptr) {
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
            if(Character *character = pParty->getCharacterByIndex(characterIndex - 1); character != nullptr) {
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
        "playSound", [](SoundId soundId) {
            pAudioPlayer->playSound(soundId, SoundPlaybackMode::SOUND_MODE_UI);
        },
        "playMusic", [](MusicId musicId) {
            pAudioPlayer->MusicPlayTrack(musicId);
        }
    );
}

void _registerItemBindings(sol::state_view &luaState, sol::table &table) {
}

void _registerSerializationBindings(sol::state_view &luaState, sol::table &table) {
    /*
    * Exposing serializations and deserializations functions to lua
    * Useful for converting command line strings to the correct types
    */
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
