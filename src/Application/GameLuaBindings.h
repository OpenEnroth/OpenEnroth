#pragma once

struct lua_State;

class GameLuaBindings {
 public:
    static void init(lua_State *lua);
};
