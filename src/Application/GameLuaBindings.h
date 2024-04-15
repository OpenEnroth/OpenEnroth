#pragma once

#include <memory>
#include <sol/forward.hpp>

struct lua_State;

class GameLuaBindings {
 public:
    GameLuaBindings();
    ~GameLuaBindings();
    void init(lua_State *lua);

 private:
    std::unique_ptr<sol::state_view> _luaState;
};
