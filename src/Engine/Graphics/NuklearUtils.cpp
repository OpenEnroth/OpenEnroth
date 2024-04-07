#include "NuklearUtils.h"

#include <string>
#include <cstring>

#include "Library/Logger/Logger.h"

int lua_check_args_count(lua_State * L, bool condition) {
    if (!condition)
        return luaL_argerror(L, -2, lua_pushfstring(L, "invalid arguments count"));

    return 0;
}

int lua_check_args(lua_State *L, bool condition) {
    if (lua_gettop(L) == 0)
        return luaL_argerror(L, 1, lua_pushfstring(L, "context is absent"));

    if (!lua_isuserdata(L, 1))
        return luaL_argerror(L, 1, lua_pushfstring(L, "context is invalid"));

    return lua_check_args_count(L, condition);
}

bool lua_to_boolean(lua_State *L, int idx) {
    bool value{};
    if (lua_isnil(L, idx)) {
        return false;
    } else if (lua_isboolean(L, idx)) {
        value = lua_toboolean(L, idx);
    } else {
        std::string_view strVal = lua_tostring(L, idx);
        value = strVal != "false" && strVal != "0";
    }

    return value;
}

bool lua_error_check(lua_State* L, int err) {
    if (err != 0) {
        logger->error("Nuklear: LUA error: {}\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        return true;
    }

    return false;
}
