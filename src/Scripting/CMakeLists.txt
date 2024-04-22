cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(SCRIPTING_SOURCES
        GameLuaBindings.cpp
        LoggerBindings.cpp
        ScriptingSystem.cpp)

set(SCRIPTING_HEADERS
        GameLuaBindings.h
        LoggerBindings.h
        LuaItemQueryTable.h
        ScriptingSystem.h)

add_library(scripting STATIC ${SCRIPTING_SOURCES} ${SCRIPTING_HEADERS})
target_check_style(scripting)

target_link_libraries(scripting
        PUBLIC
        engine
        library_logger
	    PRIVATE
	    libluajit
        sol2)
