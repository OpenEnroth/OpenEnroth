#include "GameLuaBindings.h"

#include <string>

#include <lua.hpp>

#include "Engine/Engine.h"
#include "Engine/LodTextureCache.h"
#include "Engine/GameResourceManager.h"
#include "Engine/Graphics/NuklearUtils.h"
#include "Engine/Party.h"

#include "GUI/GUIWindow.h"

void playAwardSoundsOnAllCharacters() {
    for (auto&& character : pParty->pCharacters) {
        character.PlayAwardSound_Anim();
    }
}

std::string alignmentToString(PartyAlignment alignment) {
    switch (alignment) {
    case PartyAlignment::PartyAlignment_Neutral: return "Neutral";
    case PartyAlignment::PartyAlignment_Good: return "Good";
    case PartyAlignment::PartyAlignment_Evil: return "Evil";
    }
    return "None";
}

static int lua_party_get(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 2));

    const char *name = luaL_checkstring(L, 1);
    int value;

    if (!strcmp(name, "food"))
        value = pParty->GetFood();
    else if (!strcmp(name, "gold"))
        value = pParty->GetGold();
    else
        return luaL_argerror(L, 2, lua_pushfstring(L, "name '%s' is unknown", name));

    lua_pushinteger(L, value);

    return 1;
}

static int lua_party_give(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    const char *name = luaL_checkstring(L, 1);
    int value = luaL_checknumber(L, 2);

    if (!strcmp(name, "food"))
        pParty->GiveFood(value);
    else if (!strcmp(name, "gold"))
        pParty->AddGold(value);
    else
        return luaL_argerror(L, 2, lua_pushfstring(L, "name '%s' is unknown", name));

    return 0;
}

static int lua_party_set(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    const char *name = luaL_checkstring(L, 1);
    int value = luaL_checknumber(L, 2);

    if (!strcmp(name, "food"))
        pParty->SetFood(value);
    else if (!strcmp(name, "gold"))
        pParty->SetGold(value);
    else
        return luaL_argerror(L, 2, lua_pushfstring(L, "name '%s' is unknown", name));

    return 0;
}

static int lua_party_take(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    const char *name = luaL_checkstring(L, 1);
    int value = luaL_checknumber(L, 2);

    if (!strcmp(name, "food"))
        pParty->TakeFood(value);
    else if (!strcmp(name, "gold"))
        pParty->TakeGold(value);
    else
        return luaL_argerror(L, 2, lua_pushfstring(L, "name '%s' is unknown", name));

    return 0;
}

static int lua_party_member_get(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    int playerID = luaL_checkinteger(L, 1);
    const char *name = luaL_checkstring(L, 2);
    int actual = 0, base = 0, modifier = 0;

    if (playerID < 1 || playerID > 4)
        return luaL_argerror(L, 2, lua_pushfstring(L, "member id '%d' is invalid", playerID));

    Character player = pParty->pCharacters[playerID - 1];
    if (!strcmp(name, "age")) {
        actual = player.GetActualAge();
        base = player.GetBaseAge();
        modifier = player.sAgeModifier;
    } else if (!strcmp(name, "accuracy")) {
        actual = player.GetActualAccuracy();
        base = player.GetBaseAccuracy();
        modifier = player.uAccuracyBonus;
    } else if (!strcmp(name, "armor_class")) {
        actual = player.GetActualAC();
        base = player.GetBaseAC();
        modifier = player.sACModifier;
    } else if (!strcmp(name, "attack_damage_melee")) {
        actual = player.GetActualAttack(false);
    } else if (!strcmp(name, "attack_damage_missle")) {
        actual = player.GetActualAttack(true);
    } else if (!strcmp(name, "attribute_endurance")) {
        actual = player.GetActualEndurance();
        base = player.GetBaseEndurance();
        modifier = player.uEnduranceBonus;
    } else {
        return luaL_argerror(L, 2, lua_pushfstring(L, "name '%s' is unknown", name));
    }

    lua_newtable(L);
    lua_pushliteral(L, "actual");
    lua_pushinteger(L, actual);
    lua_rawset(L, -3);
    lua_pushliteral(L, "base");
    lua_pushinteger(L, base);
    lua_rawset(L, -3);
    lua_pushliteral(L, "modifier");
    lua_pushinteger(L, modifier);
    lua_rawset(L, -3);

    return 1;
}

static int lua_load_raw_from_lod(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    const char *lod_name = luaL_checkstring(L, 1);
    const char *resource = luaL_checkstring(L, 2);
    Blob content;

    if (!strcmp(lod_name, "bitmaps"))
        content = pBitmaps_LOD->LoadCompressedTexture(resource);
    else if (!strcmp(lod_name, "events"))
        content = engine->_gameResourceManager->getEventsFile(resource);
    // else if (!strcmp(lod_name, "games"))
    //     content = pGames_LOD->LoadCompressedTexture(resource); // TODO(captainurist): temporarily commented out.
    else if (!strcmp(lod_name, "icons"))
        content = pIcons_LOD->LoadCompressedTexture(resource);
    // else if (!strcmp(lod_name, "sprites"))
    //    content = pSprites_LOD->LoadCompressedTexture(resource); // TODO(captainurist): temporarily commented out.

    if (!content)
        return luaL_argerror(L, 2, lua_pushfstring(L, "resource '%s' couldn't be loaded", resource));

    lua_pushstring(L, std::string(content.string_view()).data());

    return 1;
}

static int lua_set_game_current_menu(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 2));

    int id = luaL_checkinteger(L, 2);
    SetCurrentMenuID(MenuType(id));

    return 4;
}

static int lua_set_gold(lua_State *L) {
    lua_check_ret(lua_check_args_count(L, lua_gettop(L) == 1));

    int amount = lua_tointeger(L, 1);
    pParty->SetGold(amount);
    return 0;
}

static int lua_get_gold(lua_State *L) {
    int amount = pParty->GetGold();
    lua_pushinteger(L, amount);
    return 1;
}

static int lua_get_xp(lua_State *L) {
    lua_check_ret(lua_check_args_count(L, lua_gettop(L) == 1));

    int xp = 0;
    int characterIndex = lua_tointeger(L, 1);
    if (pParty->pCharacters.size() < characterIndex && characterIndex >= 0) {
        xp = pParty->pCharacters[characterIndex].experience;
    }

    lua_pushinteger(L, xp);
    return 1;
}

static int lua_set_xp(lua_State *L) {
    lua_check_ret(lua_check_args_count(L, lua_gettop(L) == 2));

    int characterIndex = lua_tointeger(L, 1);
    int xp = lua_tointeger(L, 2);
    if (pParty->pCharacters.size() < characterIndex && characterIndex >= 0) {
        pParty->SetCharacterXP(pParty->pCharacters[characterIndex], xp);
    }
    return 0;
}

static int lua_give_party_xp(lua_State *L) {
    lua_check_ret(lua_check_args_count(L, lua_gettop(L) == 1));
    int amount = lua_tointeger(L, 1);
    pParty->GivePartyExp(amount);
    return 0;
}

static int lua_get_character_info(lua_State *L) {
    lua_check_ret(lua_check_args_count(L, lua_gettop(L) == 1));
    int characterIndex = lua_tointeger(L, 1);
    if (characterIndex < pParty->pCharacters.size() && characterIndex >= 0) {
        auto& character = pParty->pCharacters[characterIndex];
        lua_newtable(L);
        lua_pushinteger(L, character.experience);
        lua_setfield(L, -2, "xp");

        lua_pushstring(L, character.name.c_str());
        lua_setfield(L, -2, "name");
        return 1;
    }
    return 0;
}

void GameLuaBindings::init(lua_State *lua) {
    static const luaL_Reg game[] = {
        { "load_raw_from_lod", lua_load_raw_from_lod },
        { "party_get", lua_party_get },
        { "party_give", lua_party_give },
        { "party_set", lua_party_set },
        { "party_take", lua_party_take },
        { "party_member_get", lua_party_member_get },
        { "set_current_menu", lua_set_game_current_menu },
        //newer and used by console command
        { "get_gold", lua_get_gold },
        { "set_gold", lua_set_gold },
        { "set_xp", lua_set_xp },
        { "get_xp", lua_get_xp },
        { "give_party_xp", lua_give_party_xp },
        { "get_character_info", lua_get_character_info },
        { NULL, NULL }
    };
    luaL_newlib(lua, game);
    lua_setglobal(lua, "game");
    /*
    addCommand("gold", [](int goldAmount) {
        pParty->SetGold(goldAmount);
        return commandSuccess("Set amount of gold to " + std::to_string(goldAmount));
    });

    addCommand("xp", [](int xp) {
        pParty->GivePartyExp(xp);
        playAwardSoundsOnAllCharacters();
        return commandSuccess("Added " + std::to_string(xp) + " experience points to the party.");
    });

    addCommand("sp", [](int skillpoints) {
        for (Character& character : pParty->pCharacters) {
            character.uSkillPoints += skillpoints;
        }
        playAwardSoundsOnAllCharacters();
        return commandSuccess("Added " + std::to_string(skillpoints) + " skillpoints to every character.");
    });

    addCommand("skills", []() {
        for (auto&& character : pParty->pCharacters) {
            for (CharacterSkillType skill : allSkills()) {
                // if class can learn this skill
                if (skillMaxMasteryPerClass[character.classType][skill] > CHARACTER_SKILL_MASTERY_NONE) {
                    if (character.getSkillValue(skill) == CombinedSkillValue::none()) {
                        character.setSkillValue(skill, CombinedSkillValue::novice());
                    }
                }
            }
        }
        return commandSuccess("Added all available skills to every character.");
    });

    addCommand("food", [](int foodAmount) {
        pParty->SetFood(foodAmount);
        return commandSuccess("Set amount of food to " + std::to_string(foodAmount));
    });

    addCommand("align", [](std::string alignment) {
        if (alignment.empty()) {
            return commandSuccess("Current alignment: " + alignmentToString(pParty->alignment));
        } else {
            if (alignment == "evil") { pParty->alignment = PartyAlignment::PartyAlignment_Evil;
            } else if (alignment == "good") { pParty->alignment = PartyAlignment::PartyAlignment_Good;
            } else if (alignment == "neutral") { pParty->alignment = PartyAlignment::PartyAlignment_Neutral;
            } else if (alignment == "cycle") {
                auto alignmentIndex = static_cast<int>(pParty->alignment);
                if (++alignmentIndex > static_cast<int>(PartyAlignment::PartyAlignment_Evil)) {
                    alignmentIndex = 0;
                }
                pParty->alignment = static_cast<PartyAlignment>(alignmentIndex);
            } else {
                return commandFailure("Invalid command. Choose any of: evil, good, neutral");
            }
        }

        SetUserInterface(pParty->alignment);
        return commandSuccess("Alignment changed to: " + alignmentToString(pParty->alignment));
    }, { "" });

    addCommand("wizardeye", []() {
        return toggleBooleanConfig("Wizard Eye", engine->config->debug.WizardEye);
    });

    addCommand("allmagic", []() {
        return toggleBooleanConfig("All Magic", engine->config->debug.AllMagic);
    });

    addCommand("config", [](std::string configVariableName, std::string action) {
        auto configVariable = engine->config->debug.entry(configVariableName);
        if (configVariable != nullptr) {
            if (action == "toggle") {
                return commandSuccess();
            }
        } else {
            return commandFailure("Invalid config variable. Can't find a variable of name: " + configVariableName);
        }
        return commandSuccess();
    });

    addCommand("help", []() {
        std::string result = "All Commands:\n";
        const auto& commands = engine->commandManager->getCommands();
        int index = 0;
        for (auto&& commandEntryPair : commands) {
            result += "- " + commandEntryPair.first + (index < commands.size() - 1 ? "\n" : "");
            ++index;
        }
        return commandSuccess(result);
    });
    */
}
