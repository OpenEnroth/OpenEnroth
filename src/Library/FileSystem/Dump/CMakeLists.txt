cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_FILESYSTEM_DUMP_SOURCES
        FileSystemDump.cpp)

set(LIBRARY_FILESYSTEM_DUMP_HEADERS
        FileSystemDump.h)

add_library(library_filesystem_dump STATIC ${LIBRARY_FILESYSTEM_DUMP_SOURCES} ${LIBRARY_FILESYSTEM_DUMP_HEADERS})
target_link_libraries(library_filesystem_dump PUBLIC library_filesystem_interface utility)
target_check_style(library_filesystem_dump)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_FILESYSTEM_DUMP_SOURCES Tests/FileSystemDump_ut.cpp)

    add_library(test_library_filesystem_dump OBJECT ${TEST_LIBRARY_FILESYSTEM_DUMP_SOURCES})
    target_link_libraries(test_library_filesystem_dump PUBLIC testing_unit library_filesystem_dump library_filesystem_memory library_filesystem_null)

    target_check_style(test_library_filesystem_dump)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_filesystem_dump)
endif()
