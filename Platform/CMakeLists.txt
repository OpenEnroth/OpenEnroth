cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(PLATFORM_SOURCES OSWindow.cpp
                     OSWindowFactory.cpp
                     Sdl2KeyboardController.cpp
                     Sdl2MouseController.cpp
                     Sdl2Window.cpp
                     SDLmain.cpp
)

set(PLATFORM_HEADERS Api.h
                     OSWindow.h
                     OSWindowFactory.h
                     Sdl2KeyboardController.h
                     Sdl2MouseController.h
                     Sdl2Window.h
)

if(WIN32)
    list(APPEND PLATFORM_SOURCES Win/AppSettings.cpp Win/Win.cpp)
    list(APPEND PLATFORM_HEADERS Win/Win.h)
else()
    list(APPEND PLATFORM_SOURCES Lin/Lin.cpp)
endif()

add_library(platform STATIC ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})
target_check_style(platform)

target_link_libraries(platform sdl2)
