#pragma once

#include <memory>
#include <vector>

#include "LuaItemQueryTable.h"
#include "IBindings.h"

struct lua_State;

class Character;

class GameLuaBindings : public IBindings {
 public:
    GameLuaBindings();
    ~GameLuaBindings() override;
    virtual sol::table createBindingTable(sol::state_view &solState) override;

 private:
    void _registerAudioBindings(sol::state_view &solState, sol::table &table);
    void _registerRenderBindings(sol::state_view &solState, sol::table &table);
    void _registerMiscBindings(sol::state_view &solState, sol::table &table);
    void _registerPartyBindings(sol::state_view &solState, sol::table &table);
    void _registerItemBindings(sol::state_view &solState, sol::table &table);
    void _registerSerializationBindings(sol::state_view &solState, sol::table &table);
    void _registerEnums(sol::state_view &solState, sol::table &table);

    std::unique_ptr<LuaItemQueryTable<Character>> _characterInfoQueryTable;
};
