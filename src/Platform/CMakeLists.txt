cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(PLATFORM_SOURCES
        Filters/FilteringEventHandler.cpp
        Filters/PlatformEventFilter.cpp
        PlatformEventHandler.cpp
        Proxy/ProxyEventLoop.cpp
        Proxy/ProxyGamepad.cpp
        Proxy/ProxyOpenGLContext.cpp
        Proxy/ProxyPlatform.cpp
        Proxy/ProxyWindow.cpp
        Sdl/SdlEnumTranslation.cpp
        Sdl/SdlEventLoop.cpp
        Sdl/SdlGamepad.cpp
        Sdl/SdlLogger.cpp
        Sdl/SdlMain.cpp
        Sdl/SdlOpenGLContext.cpp
        Sdl/SdlPlatform.cpp
        Sdl/SdlPlatformSharedState.cpp
        Sdl/SdlWindow.cpp)

set(PLATFORM_HEADERS
        Filters/FilteringEventHandler.h
        Filters/PlatformEventFilter.h
        Platform.h
        PlatformEnums.h
        PlatformEventHandler.h
        PlatformEventLoop.h
        PlatformEvents.h
        PlatformGamepad.h
        PlatformLogger.h
        PlatformOpenGLContext.h
        PlatformOpenGLOptions.h
        PlatformWindow.h
        Proxy/ProxyBase.h
        Proxy/ProxyEventLoop.h
        Proxy/ProxyGamepad.h
        Proxy/ProxyOpenGLContext.h
        Proxy/ProxyPlatform.h
        Proxy/ProxyWindow.h
        Sdl/SdlEnumTranslation.h
        Sdl/SdlEventLoop.h
        Sdl/SdlGamepad.h
        Sdl/SdlLogger.h
        Sdl/SdlOpenGLContext.h
        Sdl/SdlPlatform.h
        Sdl/SdlPlatformSharedState.h
        Sdl/SdlWindow.h)

if(BUILD_PLATFORM STREQUAL "windows")
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
