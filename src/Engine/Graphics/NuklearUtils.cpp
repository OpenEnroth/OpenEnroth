#include "NuklearUtils.h"

int lua_check_args_count(lua_State * L, bool condition) {
    if (!condition)
        return luaL_argerror(L, -2, lua_pushfstring(L, "invalid arguments count"));

    return 0;
}

int lua_check_args(lua_State * L, bool condition) {
    if (lua_gettop(L) == 0)
        return luaL_argerror(L, 1, lua_pushfstring(L, "context is absent"));

    if (!lua_isuserdata(L, 1))
        return luaL_argerror(L, 1, lua_pushfstring(L, "context is invalid"));

    return lua_check_args_count(L, condition);
}
