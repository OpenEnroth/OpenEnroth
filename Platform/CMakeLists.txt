cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(PLATFORM_SOURCES OSWindow.cpp)

set(PLATFORM_HEADERS Api.h
                     OSWindow.h)

if(WIN32)
    set(PLATFORM_SOURCES ${PLATFORM_SOURCES}
                         Win/WinOSWindow.cpp
                         Win/AppSettings.cpp
                         Win/Win.cpp)

    set(PLATFORM_HEADERS ${PLATFORM_HEADERS}
	                     Win/WinOSWindow.h
	                     Win/Win.h)
else()
endif()

add_library(platform STATIC ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})
target_check_style(platform)
