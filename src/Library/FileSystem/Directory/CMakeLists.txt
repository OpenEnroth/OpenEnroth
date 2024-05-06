cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_FILESYSTEM_DIRECTORY_SOURCES
        DirectoryFileSystem.cpp)

set(LIBRARY_FILESYSTEM_DIRECTORY_HEADERS
        DirectoryFileSystem.h)

add_library(library_filesystem_directory STATIC ${LIBRARY_FILESYSTEM_DIRECTORY_SOURCES} ${LIBRARY_FILESYSTEM_DIRECTORY_HEADERS})
target_link_libraries(library_filesystem_directory PUBLIC library_filesystem_interface utility)
target_check_style(library_filesystem_directory)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_FILESYSTEM_DIRECTORY_SOURCES Tests/DirectoryFileSystem_ut.cpp)

    add_library(test_library_filesystem_directory OBJECT ${TEST_LIBRARY_FILESYSTEM_DIRECTORY_SOURCES})
    target_link_libraries(test_library_filesystem_directory PUBLIC testing_unit library_filesystem_directory)

    target_check_style(test_library_filesystem_directory)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_filesystem_directory)
endif()
