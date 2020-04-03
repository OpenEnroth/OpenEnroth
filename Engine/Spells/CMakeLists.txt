cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(SPELLS_SOURCES CastSpellInfo.cpp
		   Spells.cpp)

set(SPELLS_HEADERS CastSpellInfo.h
		   Spells.h)

if(WIN32)
else()
  set(CMAKE_CXX_FLAGS "-fpermissive ${CMAKE_CXX_FLAGS}")
endif()

add_library(spells STATIC ${SPELLS_SOURCES} ${SPELLS_HEADERS})
target_check_style(spells)
