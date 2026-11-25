cmake_minimum_required(VERSION 3.27)

# Can't have git revision as a global #define, thus this lib.
#
# MM_GIT_REVISION / MM_BUILD_TIME come from BuildInfo.gen.h, regenerated on
# every build via a custom target with no OUTPUT (always out of date), not
# just on `cmake` reconfigure. See GenerateBuildInfo.cmake.

set(LIBRARY_BUILDINFO_SOURCES
        BuildInfo.cpp)

set(LIBRARY_BUILDINFO_HEADERS
        BuildInfo.h)

set(LIBRARY_BUILDINFO_GENERATED_HEADER ${CMAKE_CURRENT_BINARY_DIR}/BuildInfo.gen.h)

add_custom_target(library_buildinfo_generate
        COMMAND ${CMAKE_COMMAND}
                -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
                -DTEMPLATE=${CMAKE_CURRENT_SOURCE_DIR}/BuildInfo.gen.h.in
                -DOUTPUT=${LIBRARY_BUILDINFO_GENERATED_HEADER}
                -P ${CMAKE_SOURCE_DIR}/CMakeModules/GenerateBuildInfo.cmake
        BYPRODUCTS ${LIBRARY_BUILDINFO_GENERATED_HEADER}
        COMMENT "Regenerating BuildInfo.gen.h"
        VERBATIM)

add_library(library_buildinfo ${LIBRARY_BUILDINFO_SOURCES} ${LIBRARY_BUILDINFO_HEADERS})
add_dependencies(library_buildinfo library_buildinfo_generate)
if(TARGET check_tidy)
    add_dependencies(check_tidy library_buildinfo_generate) # BuildInfo.cpp includes the header generated here.
endif()
target_include_directories(library_buildinfo PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_check_style(library_buildinfo)
