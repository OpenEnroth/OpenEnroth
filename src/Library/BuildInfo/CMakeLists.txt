cmake_minimum_required(VERSION 3.21.1)

# Can't have git revision as a global #define, thus this lib.

set(LIBRARY_BUILDINFO_SOURCES
        BuildInfo.cpp)

set(LIBRARY_BUILDINFO_HEADERS
        BuildInfo.h)

execute_process(
        COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_REVISION
        OUTPUT_STRIP_TRAILING_WHITESPACE)

add_library(library_buildinfo ${LIBRARY_BUILDINFO_SOURCES} ${LIBRARY_BUILDINFO_HEADERS})
target_compile_definitions(library_buildinfo PRIVATE MM_GIT_REVISION="${GIT_REVISION}")
target_check_style(library_buildinfo)
