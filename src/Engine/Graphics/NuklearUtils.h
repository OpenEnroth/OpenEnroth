#pragma once

#include <lua.hpp>

#define lua_foreach(_lua, _idx) for (lua_pushnil(_lua); lua_next(_lua, _idx); lua_pop(_lua, 1))
#define lua_check_ret(_func) { int _ret = _func; if (_ret) return _ret; }

int lua_check_args_count(lua_State *L, bool condition);
int lua_check_args(lua_State *L, bool condition);
bool lua_to_boolean(lua_State *L, int idx);
bool lua_error_check(lua_State * L, int err);
