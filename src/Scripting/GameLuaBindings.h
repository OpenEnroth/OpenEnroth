#pragma once

#include <memory>
#include <vector>

#include "LuaItemQueryTable.h"
#include "IBindings.h"

struct lua_State;

class Character;

class GameLuaBindings : public IBindings {
 public:
    explicit GameLuaBindings(const sol::state_view &luaState);
    ~GameLuaBindings() override;
    sol::table getBindingTable() override;

 private:
    void _registerAudioBindings(sol::table &table);
    void _registerRenderBindings(sol::table &table);
    void _registerMiscBindings(sol::table &table);
    void _registerPartyBindings(sol::table &table);
    void _registerItemBindings(sol::table &table);
    void _registerSerializationBindings(sol::table &table);
    void _registerEnums(sol::table &table);

    sol::state_view _luaState;
    std::optional<sol::table> _bindingTable;
    LuaItemQueryTable<Character> _characterInfoQueryTable;
};
