cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

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

if(OE_BUILD_TESTS)
    set(TEST_ENGINE_SPELLS_SOURCES Tests/Spells_ut.cpp)
    add_library(test_engine_spells OBJECT ${TEST_ENGINE_SPELLS_SOURCES})
    target_link_libraries(test_engine_spells PUBLIC testing_unit engine_spells)
    target_check_style(test_engine_spells)
    target_link_libraries(OpenEnroth_GameTest PUBLIC test_engine_spells)
endif()
