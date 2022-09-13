cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

include_directories(${INCLUDE_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR})

set(LAUNCHER_SOURCES src/Application.cpp
                     src/Launcher.cpp)

set(LAUNCHER_HEADERS resource.h
                     src/Application.h
                     src/ApplicationConfig.h)

set(LAUNCHER_RESOURCES ico128.ico
                       Resource.aps
                       Resource.rc
                       shield-flip.bmp
                       shield.bmp)

add_library(Launcher STATIC ${LAUNCHER_SOURCES} ${LAUNCHER_HEADERS})
