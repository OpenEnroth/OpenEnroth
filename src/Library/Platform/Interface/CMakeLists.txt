cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_PLATFORM_INTERFACE_SOURCES
        PlatformEventHandler.cpp)

set(LIBRARY_PLATFORM_INTERFACE_HEADERS
        Platform.h
        PlatformEnums.h
        PlatformEventHandler.h
        PlatformEventLoop.h
        PlatformEvents.h
        PlatformGamepad.h
        PlatformOpenGLContext.h
        PlatformOpenGLOptions.h
        PlatformWindow.h)

add_library(library_platform_interface STATIC ${LIBRARY_PLATFORM_INTERFACE_SOURCES} ${LIBRARY_PLATFORM_INTERFACE_HEADERS})
target_check_style(library_platform_interface)
target_link_libraries(library_platform_interface PUBLIC utility library_logger library_geometry)
