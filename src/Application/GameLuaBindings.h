#pragma once

#include <memory>
#include <vector>
#include <sol/sol.hpp>

#include "LuaItemQueryTable.h"

struct lua_State;

class Character;

class GameLuaBindings {
 public:
    GameLuaBindings();
    ~GameLuaBindings();
    void init(lua_State *lua);

 private:
    void _registerAudioBindings(sol::state_view &luaState, sol::table &table);
    void _registerRenderBindings(sol::state_view &luaState, sol::table &table);
    void _registerGameBindings(sol::state_view &luaState, sol::table &table);
    void _registerPartyBindings(sol::state_view &luaState, sol::table &table);
    void _registerItemBindings(sol::state_view &luaState, sol::table &table);
    void _registerSerializationBindings(sol::state_view &luaState, sol::table &table);
    void _registerEnums(sol::state_view &luaState, sol::table &table);

    std::unique_ptr<sol::state_view> _luaState;
    std::unique_ptr<LuaItemQueryTable<Character>> _characterInfoQueryTable;
};
