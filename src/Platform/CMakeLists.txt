cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(PLATFORM_SOURCES PlatformKey.cpp
                     PlatformEventHandler.cpp
                     Sdl/SdlEnumTranslation.cpp
                     Sdl/SdlEventLoop.cpp
                     Sdl/SdlMain.cpp
                     Sdl/SdlOpenGLContext.cpp
                     Sdl/SdlPlatform.cpp
                     Sdl/SdlPlatformSharedState.cpp
                     Sdl/SdlWindow.cpp
)

set(PLATFORM_HEADERS Platform.h
                     PlatformKey.h
                     PlatformModifiers.h
                     PlatformMouseButton.h
                     PlatformEventHandler.h
                     PlatformEventLoop.h
                     PlatformEvents.h
                     PlatformWindow.h
                     PlatformOpenGLContext.h
                     PlatformOpenGLOptions.h
                     Sdl/SdlEnumTranslation.h
                     Sdl/SdlEventLoop.h
                     Sdl/SdlOpenGLContext.h
                     Sdl/SdlPlatform.h
                     Sdl/SdlPlatformSharedState.h
                     Sdl/SdlWindow.h
)

if(WIN32)
  list(APPEND PLATFORM_SOURCES Win/WinPlatform.cpp)
  list(APPEND PLATFORM_HEADERS Win/WinPlatform.h)
else()
  list(APPEND PLATFORM_SOURCES Posix/PosixPlatform.cpp)
endif()

include_directories(${INCLUDE_DIRECTORIES} ${SDL2_INCLUDE_DIRS})

add_library(platform STATIC ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})
target_check_style(platform)

target_link_libraries(platform ${SDL2_LIBRARIES})

message(VERBOSE "SDL2_LIBRARIES: ${SDL2_LIBRARIES}")
