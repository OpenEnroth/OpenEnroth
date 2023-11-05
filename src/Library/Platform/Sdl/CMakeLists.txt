cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(PLATFORM_SDL_SOURCES
        SdlEnumTranslation.cpp
        SdlEventLoop.cpp
        SdlGamepad.cpp
        SdlLogSource.cpp
        SdlOpenGLContext.cpp
        SdlPlatform.cpp
        SdlPlatformSharedState.cpp
        SdlWindow.cpp)

set(PLATFORM_SDL_HEADERS
        SdlEnumTranslation.h
        SdlEventLoop.h
        SdlGamepad.h
        SdlLogSource.h
        SdlOpenGLContext.h
        SdlPlatform.h
        SdlPlatformSharedState.h
        SdlWindow.h)

set(PLATFORM_SDL_MAIN_SOURCES
        SdlMain.cpp)

set(PLATFORM_SDL_MAIN_HEADERS)

# Need to have main in an OBJECT lib so that it's properly exported from Android .so.
add_library(library_platform_main OBJECT ${PLATFORM_SDL_MAIN_SOURCES} ${PLATFORM_SDL_MAIN_HEADERS})
target_check_style(library_platform_main)
target_link_libraries(library_platform_main PRIVATE SDL2::SDL2OE)

add_library(library_platform_sdl STATIC ${PLATFORM_SDL_SOURCES} ${PLATFORM_SDL_HEADERS})
target_check_style(library_platform_sdl)
target_link_libraries(library_platform_sdl PUBLIC utility library_logger PRIVATE SDL2::SDL2OE)
