cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(PLATFORM_SOURCES PlatformEventHandler.cpp
                     Filters/FilteringEventHandler.cpp
                     Filters/PlatformEventFilter.cpp
                     Sdl/SdlEnumTranslation.cpp
                     Sdl/SdlEventLoop.cpp
                     Sdl/SdlMain.cpp
                     Sdl/SdlOpenGLContext.cpp
                     Sdl/SdlPlatform.cpp
                     Sdl/SdlPlatformSharedState.cpp
                     Sdl/SdlWindow.cpp
                     Sdl/SdlLogger.cpp
                     Sdl/SdlGamepad.cpp
                     Proxy/ProxyPlatform.cpp
                     Proxy/ProxyWindow.cpp
                     Proxy/ProxyEventLoop.cpp
                     Proxy/ProxyOpenGLContext.cpp
                     Proxy/ProxyGamepad.cpp)

set(PLATFORM_HEADERS Platform.h
                     PlatformEnums.h
                     PlatformEventHandler.h
                     PlatformEventLoop.h
                     PlatformEvents.h
                     PlatformLogger.h
                     PlatformWindow.h
                     PlatformGamepad.h
                     PlatformOpenGLContext.h
                     PlatformOpenGLOptions.h
                     Filters/FilteringEventHandler.h
                     Filters/PlatformEventFilter.h
                     Sdl/SdlEnumTranslation.h
                     Sdl/SdlEventLoop.h
                     Sdl/SdlOpenGLContext.h
                     Sdl/SdlPlatform.h
                     Sdl/SdlPlatformSharedState.h
                     Sdl/SdlWindow.h
                     Sdl/SdlLogger.h
                     Sdl/SdlGamepad.h
                     Proxy/ProxyPlatform.h
                     Proxy/ProxyWindow.h
                     Proxy/ProxyEventLoop.h
                     Proxy/ProxyOpenGLContext.h
                     Proxy/ProxyGamepad.h
                     Proxy/ProxyBase.h)

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
