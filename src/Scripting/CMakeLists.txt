cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(SCRIPTING_SOURCES
        GameLuaBindings.cpp)

set(SCRIPTING_HEADERS
        GameLuaBindings.h
        LuaItemQueryTable.h)

add_library(scripting STATIC ${SCRIPTING_SOURCES} ${SCRIPTING_HEADERS})
target_check_style(scripting)

target_link_libraries(scripting
        PUBLIC
        engine
	    PRIVATE
	    libluajit
        sol2)
