cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(PLATFORM_SOURCES
        Filters/FilteringEventHandler.cpp
        Filters/PlatformEventFilter.cpp
        Null/NullEventLoop.cpp
        Null/NullOpenGLContext.cpp
        Null/NullPlatform.cpp
        Null/NullWindow.cpp
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
        Sdl/SdlOpenGLContext.cpp
        Sdl/SdlPlatform.cpp
        Sdl/SdlPlatformSharedState.cpp
        Sdl/SdlWindow.cpp)

set(PLATFORM_HEADERS
        Filters/FilteringEventHandler.h
        Filters/PlatformEventFilter.h
        Null/NullEventLoop.h
        Null/NullOpenGLContext.h
        Null/NullPlatform.h
        Null/NullPlatformOptions.h
        Null/NullPlatformSharedState.h
        Null/NullWindow.h
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

set(PLATFORM_MAIN_SOURCES
        Sdl/SdlMain.cpp)

set(PLATFORM_MAIN_HEADERS)

# Need to have main in an OBJECT lib so that it's properly exported from Android .so.
add_library(platform_main OBJECT ${PLATFORM_MAIN_SOURCES} ${PLATFORM_MAIN_HEADERS})
target_check_style(platform_main)
target_link_libraries(platform_main PRIVATE SDL2::SDL2OE)

add_library(platform STATIC ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})
target_check_style(platform)
target_link_libraries(platform PRIVATE SDL2::SDL2OE)
