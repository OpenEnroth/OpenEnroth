cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_FILESYSTEM_MERGING_SOURCES
        MergingFileSystem.cpp)

set(LIBRARY_FILESYSTEM_MERGING_HEADERS
        MergingFileSystem.h)

add_library(library_filesystem_merging STATIC ${LIBRARY_FILESYSTEM_MERGING_SOURCES} ${LIBRARY_FILESYSTEM_MERGING_HEADERS})
target_link_libraries(library_filesystem_merging PUBLIC library_filesystem_interface library_filesystem_null utility)
target_check_style(library_filesystem_merging)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_FILESYSTEM_MERGING_SOURCES Tests/MergingFileSystem_ut.cpp)

    add_library(test_library_filesystem_merging OBJECT ${TEST_LIBRARY_FILESYSTEM_MERGING_SOURCES})
    target_link_libraries(test_library_filesystem_merging PUBLIC testing_unit library_filesystem_merging library_filesystem_memory library_filesystem_dump)

    target_check_style(test_library_filesystem_merging)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_filesystem_merging)
endif()
