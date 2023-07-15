cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(BIN_OPENENROTH_SOURCES
        OpenEnroth.cpp
        OpenEnrothOptions.cpp)

set(BIN_OPENENROTH_HEADERS
        OpenEnrothOptions.h)

if(BUILD_PLATFORM STREQUAL "android")
    add_library(main SHARED)
    target_sources(main PUBLIC ${BIN_OPENENROTH_HEADERS} ${BIN_OPENENROTH_SOURCES})
    target_check_style(main)
    target_link_libraries(main application)
else()
    add_executable(OpenEnroth MACOSX_BUNDLE WIN32)
    target_sources(OpenEnroth PUBLIC ${BIN_OPENENROTH_HEADERS} ${BIN_OPENENROTH_SOURCES})
    target_check_style(OpenEnroth)
    target_fix_libcxx_assertions(OpenEnroth)

    target_link_libraries(OpenEnroth application)

    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenEnroth)

    PREBUILT_DEPENDENCIES_RESOLVE(OpenEnroth)
endif()
