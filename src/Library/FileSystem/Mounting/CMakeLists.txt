cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_FILESYSTEM_MOUNTING_SOURCES
        MountingFileSystem.cpp)

set(LIBRARY_FILESYSTEM_MOUNTING_HEADERS
        MountingFileSystem.h)

add_library(library_filesystem_mounting STATIC ${LIBRARY_FILESYSTEM_MOUNTING_SOURCES} ${LIBRARY_FILESYSTEM_MOUNTING_HEADERS})
target_link_libraries(library_filesystem_mounting PUBLIC library_filesystem_interface utility)
target_check_style(library_filesystem_mounting)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_FILESYSTEM_MOUNTING_SOURCES Tests/MountingFileSystem_ut.cpp)

    add_library(test_library_filesystem_mounting OBJECT ${TEST_LIBRARY_FILESYSTEM_MOUNTING_SOURCES})
    target_link_libraries(test_library_filesystem_mounting PUBLIC
            testing_unit
            library_filesystem_mounting
            library_filesystem_memory
            library_filesystem_null
            library_filesystem_merging
            library_filesystem_dump)

    target_check_style(test_library_filesystem_mounting)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_filesystem_mounting)
endif()
