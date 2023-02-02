cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(APPLICATION_SOURCES PlatformApplication.cpp)

set(APPLICATION_HEADERS PlatformApplication.h
                        PlatformApplicationAware.h
                        PlatformIntrospection.h)

add_library(application STATIC ${APPLICATION_SOURCES} ${APPLICATION_HEADERS})
target_link_libraries(application platform)
target_check_style(application)
