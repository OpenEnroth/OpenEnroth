#pragma once

#include <memory>

struct lua_State;

namespace sol {
class state_view;
}

class GameLuaBindings {
 public:
    GameLuaBindings();
    ~GameLuaBindings();
    void init(lua_State *lua);

 private:
    std::unique_ptr<sol::state_view> _luaState;
};
