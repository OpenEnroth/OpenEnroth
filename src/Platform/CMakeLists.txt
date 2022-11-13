cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(PLATFORM_SOURCES OSWindow.cpp
                     OSWindowFactory.cpp
                     Sdl2KeyboardController.cpp
                     Sdl2Window.cpp
                     SDLmain.cpp
                     PlatformKey.cpp
)

set(PLATFORM_HEADERS Api.h
                     PlatformKey.h
                     OSWindow.h
                     OSWindowFactory.h
                     Sdl2KeyboardController.h
                     Sdl2Window.h
)

if(WIN32)
  list(APPEND PLATFORM_SOURCES Win/AppSettings.cpp
                               Win/Win.cpp)
  list(APPEND PLATFORM_HEADERS Win/Win.h)
else()
  list(APPEND PLATFORM_SOURCES Posix/Posix.cpp)
  list(APPEND PLATFORM_HEADERS Posix/Posix.h)
endif()

include_directories(${INCLUDE_DIRECTORIES} ${SDL2_INCLUDE_DIRS})

add_library(platform STATIC ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})
target_check_style(platform)

target_link_libraries(platform ${SDL2_LIBRARIES})

message(VERBOSE "SDL2_LIBRARIES: ${SDL2_LIBRARIES}")
