cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(TOOLS_SOURCES
    MemBuffer.cpp
)

set(TOOLS_HEADERS
    MemBuffer.h
)

if(WIN32)
else()
    set(CMAKE_CXX_FLAGS "-fpermissive ${CMAKE_CXX_FLAGS}")
endif()

add_library(tools STATIC ${TOOLS_SOURCES} ${TOOLS_HEADERS})

target_check_style(tools)
