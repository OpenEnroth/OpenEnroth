cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_FILESYSTEM_MASKING_SOURCES
        MaskingFileSystem.cpp)

set(LIBRARY_FILESYSTEM_MASKING_HEADERS
        MaskingFileSystem.h)

add_library(library_filesystem_masking STATIC ${LIBRARY_FILESYSTEM_MASKING_SOURCES} ${LIBRARY_FILESYSTEM_MASKING_HEADERS})
target_link_libraries(library_filesystem_masking PUBLIC library_filesystem_interface library_filesystem_proxy utility)
target_check_style(library_filesystem_masking)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_FILESYSTEM_MASKING_SOURCES Tests/MaskingFileSystem_ut.cpp)

    add_library(test_library_filesystem_masking OBJECT ${TEST_LIBRARY_FILESYSTEM_MASKING_SOURCES})
    target_link_libraries(test_library_filesystem_masking PUBLIC testing_unit library_filesystem_masking library_filesystem_memory)

    target_check_style(test_library_filesystem_masking)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_filesystem_masking)
endif()
