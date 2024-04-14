#include "GameLuaBindings.h"

#include <string_view>
#include <memory>
#include <sol/sol.hpp>

#include "Engine/Party.h"
#include "Library/Logger/Logger.h"

#include "GUI/GUIWindow.h"

GameLuaBindings::GameLuaBindings() = default;
GameLuaBindings::~GameLuaBindings() = default;

void GameLuaBindings::init(lua_State *L) {
    _luaState = std::make_unique<sol::state_view>(L);

    _luaState->create_named_table(
        "game",
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
            return toString(pParty->alignment);
        },
        "set_alignment", [](std::string_view align) {
            PartyAlignment alignment;
            if (tryDeserialize(align, &alignment)) {
                pParty->alignment = alignment;
                SetUserInterface(pParty->alignment);
            }
        },
        "give_party_xp", [](int amount) {
            pParty->GivePartyExp(amount);
        },
        "get_party_size", []() {
            return pParty->pCharacters.size();
        },
        "get_character_info", [luaState = _luaState.get()](int characterIndex) {
            if(Character *character = pParty->getCharacterByIndex(characterIndex - 1); character != nullptr) {
                return luaState->create_table_with(
                    "name", character->name,
                    "xp", character->experience,
                    "sp", character->uSkillPoints
                );
            }
            return luaState->create_table();
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
