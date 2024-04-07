#include "GameLuaBindings.h"

#include <string>

#include <lua.hpp>

#include "Engine/Engine.h"
#include "Engine/LodTextureCache.h"
#include "Engine/GameResourceManager.h"
#include "Engine/Graphics/NuklearUtils.h"
#include "Engine/Party.h"

#include "GUI/GUIWindow.h"

static int lua_check_character_index(lua_State* L, int idx, Character *&value) {
    int characterIndex = lua_tointeger(L, idx);
    if (characterIndex - 1 >= pParty->pCharacters.size() || characterIndex - 1 < 0) {
        return luaL_argerror(L, idx, lua_pushfstring(L, "Invalid character index: '%d'", characterIndex));
    }
    value = &pParty->pCharacters[characterIndex - 1];
    return 0;
}

static int lua_party_get(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 2));

    std::string_view name = luaL_checkstring(L, 1);
    int value;

    if (name == "food")
        value = pParty->GetFood();
    else if (name == "gold")
        value = pParty->GetGold();
    else
        return luaL_argerror(L, 2, lua_pushfstring(L, "name '%s' is unknown", name));

    lua_pushinteger(L, value);

    return 1;
}

static int lua_party_give(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    std::string_view name = luaL_checkstring(L, 1);
    int value = luaL_checknumber(L, 2);

    if (name == "food")
        pParty->GiveFood(value);
    else if (name == "gold")
        pParty->AddGold(value);
    else
        return luaL_argerror(L, 2, lua_pushfstring(L, "name '%s' is unknown", name));

    return 0;
}

static int lua_party_set(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    std::string_view name = luaL_checkstring(L, 1);
    int value = luaL_checknumber(L, 2);

    if (name == "food")
        pParty->SetFood(value);
    else if (name == "gold")
        pParty->SetGold(value);
    else
        return luaL_argerror(L, 2, lua_pushfstring(L, "name '%s' is unknown", name));

    return 0;
}

static int lua_party_take(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    std::string_view name = luaL_checkstring(L, 1);
    int value = luaL_checknumber(L, 2);

    if (name == "food")
        pParty->TakeFood(value);
    else if (name == "gold")
        pParty->TakeGold(value);
    else
        return luaL_argerror(L, 2, lua_pushfstring(L, "name '%s' is unknown", name));

    return 0;
}

static int lua_party_member_get(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    int playerID = luaL_checkinteger(L, 1);
    std::string_view name = luaL_checkstring(L, 2);
    int actual = 0, base = 0, modifier = 0;

    if (playerID < 1 || playerID > 4)
        return luaL_argerror(L, 2, lua_pushfstring(L, "member id '%d' is invalid", playerID));

    Character player = pParty->pCharacters[playerID - 1];
    if (name == "age") {
        actual = player.GetActualAge();
        base = player.GetBaseAge();
        modifier = player.sAgeModifier;
    } else if (name == "accuracy") {
        actual = player.GetActualAccuracy();
        base = player.GetBaseAccuracy();
        modifier = player.uAccuracyBonus;
    } else if (name == "armor_class") {
        actual = player.GetActualAC();
        base = player.GetBaseAC();
        modifier = player.sACModifier;
    } else if (name == "attack_damage_melee") {
        actual = player.GetActualAttack(false);
    } else if (name == "attack_damage_missile") {
        actual = player.GetActualAttack(true);
    } else if (name == "attribute_endurance") {
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

    std::string_view lod_name = luaL_checkstring(L, 1);
    const char* resource = luaL_checkstring(L, 2);
    Blob content;

    if (lod_name == "bitmaps")
        content = pBitmaps_LOD->LoadCompressedTexture(resource);
    else if (lod_name == "events")
        content = engine->_gameResourceManager->getEventsFile(resource);
    // else if (!strcmp(lod_name, "games"))
    //     content = pGames_LOD->LoadCompressedTexture(resource); // TODO(captainurist): temporarily commented out.
    else if (lod_name == "icons")
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

static int lua_set_food(lua_State *L) {
    lua_check_ret(lua_check_args_count(L, lua_gettop(L) == 1));

    int amount = lua_tointeger(L, 1);
    pParty->SetFood(amount);
    return 0;
}

static int lua_get_food(lua_State *L) {
    int amount = pParty->GetFood();
    lua_pushinteger(L, amount);
    return 1;
}

static int lua_give_party_xp(lua_State *L) {
    lua_check_ret(lua_check_args_count(L, lua_gettop(L) == 1));
    int amount = lua_tointeger(L, 1);
    pParty->GivePartyExp(amount);
    return 0;
}

static int lua_get_character_info(lua_State *L) {
    lua_check_ret(lua_check_args_count(L, lua_gettop(L) == 1));
    Character* character;
    lua_check_ret(lua_check_character_index(L, 1, character));

    lua_newtable(L);
    lua_pushinteger(L, character->experience);
    lua_setfield(L, -2, "xp");
    lua_pushinteger(L, character->uSkillPoints);
    lua_setfield(L, -2, "sp");

    lua_pushstring(L, character->name.c_str());
    lua_setfield(L, -2, "name");
    return 1;
}

static int lua_set_character_info(lua_State *L) {
    lua_check_ret(lua_check_args_count(L, lua_gettop(L) == 2));
    Character* character;
    lua_check_ret(lua_check_character_index(L, 1, character));

    if (lua_istable(L, 2)) {
        lua_pushnil(L);
        while (lua_next(L, 2)) {
            std::string_view key = lua_tostring(L, -2);
            if (key, "xp") {
                character->setXP(lua_tointeger(L, -1));
            } else if (key == "sp") {
                auto val = lua_tointeger(L, -1);
                character->uSkillPoints = val < 0 ? 0 : val;
            } else {
                return luaL_argerror(L, 2, lua_pushfstring(L, "Invalid key for character info: '%s'", key));
            }
            lua_pop(L, 1);
        }
    } else {
        return luaL_argerror(L, 2, lua_pushfstring(L, "Expected a table to set character info"));
    }

    return 0;
}

static int lua_play_character_award_sound(lua_State *L) {
    lua_check_ret(lua_check_args_count(L, lua_gettop(L) == 1));
    Character* character{};
    lua_check_ret(lua_check_character_index(L, 1, character));
    character->PlayAwardSound_Anim();
    return 0;
}

static int lua_play_all_characters_award_sound(lua_State *L) {
    for (auto&& character : pParty->pCharacters) {
        character.PlayAwardSound_Anim();
    }
    return 0;
}

static int lua_get_party_size(lua_State *L) {
    lua_pushinteger(L, pParty->pCharacters.size());
    return 1;
}

static int lua_set_alignment(lua_State* L) {
    lua_check_ret(lua_check_args_count(L, lua_gettop(L) == 1));
    std::string_view alignmentStr = lua_tostring(L, 1);
    PartyAlignment alignment;
    if (tryDeserialize(alignmentStr, &alignment)) {
        pParty->alignment = alignment;
        SetUserInterface(pParty->alignment);
    } else {
        return luaL_argerror(L, 1, lua_pushfstring(L, "Invalid alignment value: '%s'", alignmentStr));
    }
    return 0;
}

static int lua_get_alignment(lua_State* L) {
    std::string alignmentStr;
    serialize(pParty->alignment, &alignmentStr);
    lua_pushstring(L, alignmentStr.c_str());
    return 1;
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
        { "get_food", lua_get_food },
        { "set_food", lua_set_food },
        { "give_party_xp", lua_give_party_xp },
        { "get_party_size", lua_get_party_size },
        { "get_character_info", lua_get_character_info },
        { "set_character_info", lua_set_character_info },
        { "play_character_award_sound", lua_play_character_award_sound },
        { "play_all_characters_award_sound", lua_play_all_characters_award_sound },
        { "set_alignment", lua_set_alignment },
        { "get_alignment", lua_get_alignment },
        { NULL, NULL }
    };
    luaL_newlib(lua, game);
    lua_setglobal(lua, "game");
}
