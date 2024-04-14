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
    void _registerAudioBindings();
    void _registerGameBindings();
    void _registerSerializationBindings();

    std::unique_ptr<sol::state_view> _luaState;
};
