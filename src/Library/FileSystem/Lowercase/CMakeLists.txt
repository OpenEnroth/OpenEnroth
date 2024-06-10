cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_FILESYSTEM_LOWERCASE_SOURCES
        LowercaseFileSystem.cpp)

set(LIBRARY_FILESYSTEM_LOWERCASE_HEADERS
        LowercaseFileSystem.h)

add_library(library_filesystem_lowercase STATIC ${LIBRARY_FILESYSTEM_LOWERCASE_SOURCES} ${LIBRARY_FILESYSTEM_LOWERCASE_HEADERS})
target_link_libraries(library_filesystem_lowercase PUBLIC library_filesystem_interface library_filesystem_trie utility)
target_check_style(library_filesystem_lowercase)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_FILESYSTEM_LOWERCASE_SOURCES Tests/LowercaseFileSystem_ut.cpp)

    add_library(test_library_filesystem_lowercase OBJECT ${TEST_LIBRARY_FILESYSTEM_LOWERCASE_SOURCES})
    target_link_libraries(test_library_filesystem_lowercase PUBLIC testing_unit library_filesystem_lowercase library_filesystem_memory library_filesystem_directory)

    target_check_style(test_library_filesystem_lowercase)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_filesystem_lowercase)
endif()
