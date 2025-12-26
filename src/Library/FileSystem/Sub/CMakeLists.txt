cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_FILESYSTEM_SUB_SOURCES
        SubFileSystem.cpp)

set(LIBRARY_FILESYSTEM_SUB_HEADERS
        SubFileSystem.h)

add_library(library_filesystem_sub STATIC ${LIBRARY_FILESYSTEM_SUB_SOURCES} ${LIBRARY_FILESYSTEM_SUB_HEADERS})
target_link_libraries(library_filesystem_sub PUBLIC library_filesystem_interface)
target_check_style(library_filesystem_sub)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_FILESYSTEM_SUB_SOURCES Tests/SubFileSystem_ut.cpp)

    add_library(test_library_filesystem_sub OBJECT ${TEST_LIBRARY_FILESYSTEM_SUB_SOURCES})
    target_link_libraries(test_library_filesystem_sub PUBLIC testing_unit library_filesystem_sub library_filesystem_memory)
    target_check_style(test_library_filesystem_sub)
    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_filesystem_sub)
endif()
