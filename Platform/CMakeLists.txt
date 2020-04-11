cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(PLATFORM_SOURCES OSWindow.cpp
                     OSWindowFactory.cpp
                     Sdl2Window.cpp)

set(PLATFORM_HEADERS Api.h
                     OSWindow.h
                     OSWindowFactory.h
                     Sdl2Window.h)

if(WIN32)
    list(APPEND PLATFORM_SOURCES Win/AppSettings.cpp Win/Win.cpp)
    list(APPEND PLATFORM_HEADERS Win/Win.h)
else()
    list(APPEND PLATFORM_SOURCES Lin/Lin.cpp)
endif()

add_library(platform STATIC ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})
target_check_style(platform)

target_link_libraries(platform sdl2)
