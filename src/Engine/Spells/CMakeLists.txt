cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(ENGINE_SPELLS_SOURCES
        CastSpellInfo.cpp
        Spells.cpp)

set(ENGINE_SPELLS_HEADERS
        CastSpellInfo.h
        SpellBuff.h
        SpellEnums.h
        SpellEnumFunctions.h
        Spells.h)

add_library(engine_spells STATIC ${ENGINE_SPELLS_SOURCES} ${ENGINE_SPELLS_HEADERS})
target_link_libraries(engine_spells PUBLIC engine)
target_check_style(engine_spells)
