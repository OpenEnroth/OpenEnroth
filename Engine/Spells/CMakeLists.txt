cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(SPELLS_SOURCES CastSpellInfo.cpp
		   Spells.cpp)

set(SPELLS_HEADERS CastSpellInfo.h
		   Spells.h)

if(WIN32)
else()
endif()

add_library(spells STATIC ${SPELLS_SOURCES} ${SPELLS_HEADERS})
target_check_style(spells)
