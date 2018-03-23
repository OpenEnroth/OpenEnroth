cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(PLATFORM_SOURCES OSWindow.cpp
                     Win/AppSettings.cpp
                     Win/Win.cpp)

set(PLATFORM_HEADERS Api.h
                     OSWindow.h
                     Win/Win.h)

if(WIN32)
else()
endif()

add_library(platform STATIC ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})
target_check_style(platform)
