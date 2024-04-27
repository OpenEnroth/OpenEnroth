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
    virtual sol::table createBindingTable(sol::state_view &solState) const override;

 private:
    void _registerAudioBindings(sol::state_view &solState, sol::table &table) const;
    void _registerRenderBindings(sol::state_view &solState, sol::table &table) const;
    void _registerMiscBindings(sol::state_view &solState, sol::table &table) const;
    void _registerPartyBindings(sol::state_view &solState, sol::table &table) const;
    void _registerItemBindings(sol::state_view &solState, sol::table &table) const;
    void _registerSerializationBindings(sol::state_view &solState, sol::table &table) const;
    void _registerEnums(sol::state_view &solState, sol::table &table) const;

    /* TODO(Gerark) This variable becoming a static is just a temporary hack
     * Needs to be moved to another place where the ScriptingSystem is providing binding helpers instead */
    static std::unique_ptr<LuaItemQueryTable<Character>> _characterInfoQueryTable;
};
