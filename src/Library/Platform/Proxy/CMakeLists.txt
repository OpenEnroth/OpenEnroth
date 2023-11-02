cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(LIBRARY_PLATFORM_PROXY_SOURCES
        ProxyEventLoop.cpp
        ProxyGamepad.cpp
        ProxyOpenGLContext.cpp
        ProxyPlatform.cpp
        ProxyWindow.cpp)

set(LIBRARY_PLATFORM_PROXY_HEADERS
        ProxyBase.h
        ProxyEventLoop.h
        ProxyGamepad.h
        ProxyOpenGLContext.h
        ProxyPlatform.h
        ProxyWindow.h)

add_library(library_platform_proxy STATIC ${LIBRARY_PLATFORM_PROXY_SOURCES} ${LIBRARY_PLATFORM_PROXY_HEADERS})
target_check_style(library_platform_proxy)
target_link_libraries(library_platform_proxy PUBLIC library_platform_interface)
