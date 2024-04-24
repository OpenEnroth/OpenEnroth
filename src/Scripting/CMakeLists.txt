cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(SCRIPTING_SOURCES
        ConfigBindings.cpp
        GameLuaBindings.cpp
        InputBindings.cpp
        InputScriptEventHandler.cpp
        LoggerBindings.cpp
        NuklearBindings.cpp
        ScriptingSystem.cpp
        ScriptLogSink.cpp)

set(SCRIPTING_HEADERS
        ConfigBindings.h
        GameLuaBindings.h
        InputBindings.h
        InputScriptEventHandler.h
        LoggerBindings.h
        NuklearBindings.h
        LuaItemQueryTable.h
        ScriptingSystem.h
        ScriptLogSink.h)

add_library(scripting STATIC ${SCRIPTING_SOURCES} ${SCRIPTING_HEADERS})
target_check_style(scripting)

target_link_libraries(scripting
        PUBLIC
        engine
        library_logger
	    PRIVATE
	    libluajit
        sol2)
