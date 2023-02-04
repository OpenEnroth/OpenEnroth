cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(SPELLS_SOURCES CastSpellInfo.cpp
                   Spells.cpp)

set(SPELLS_HEADERS CastSpellInfo.h
                   Spells.h)

add_library(spells STATIC ${SPELLS_SOURCES} ${SPELLS_HEADERS})
target_link_libraries(spells engine)
target_check_style(spells)
