#include "NuklearLogSink.h"

#include <string>
#include <lua.hpp>

#include "NuklearUtils.h"
#include "Nuklear.h"

void NuklearLogSink::write(const LogCategory& category, LogLevel level, std::string_view message) {
    if (_isLogging) {
        return; //early return to avoid potential infinite recursion if another log message is raised from lua
    }
    _isLogging = true;

    lua_State* lua = Nuklear::getLuaState();

    lua_getfield(lua, LUA_GLOBALSINDEX, "logsink");
    if (lua_isnil(lua, -1)) {
        lua_pop(lua, 1);
        _isLogging = false;
        return;
    }
    std::string serializedLevel;
    serialize(level, &serializedLevel);
    lua_pushstring(lua, serializedLevel.c_str());
    lua_pushstring(lua, message.data());
    int error = lua_pcall(lua, 2, 0, 0);
    lua_error_check(lua, error);
    _isLogging = false;
}

std::unique_ptr<LogSink> NuklearLogSink::createNuklearLogSink() {
    return std::make_unique<NuklearLogSink>();
}
