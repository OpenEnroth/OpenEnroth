cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

if(BUILD_PLATFORM STREQUAL "android")
    add_library(main SHARED)
    target_sources(main PUBLIC main.cpp)
    target_check_style(main)
    target_link_libraries(main application)
else()
    add_executable(OpenEnroth MACOSX_BUNDLE WIN32 main.cpp)
    target_check_style(OpenEnroth)
    target_fix_libcxx_assertions(OpenEnroth)

    target_link_libraries(OpenEnroth application)

    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenEnroth)

    PREBUILT_DEPENDENCIES_RESOLVE(OpenEnroth)
endif()
