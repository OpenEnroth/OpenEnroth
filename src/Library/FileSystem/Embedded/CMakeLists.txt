cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_FILESYSTEM_EMBEDDED_SOURCES
        EmbeddedFileSystem.cpp)

set(LIBRARY_FILESYSTEM_EMBEDDED_HEADERS
        EmbeddedFileSystem.h)

add_library(library_filesystem_embedded STATIC ${LIBRARY_FILESYSTEM_EMBEDDED_SOURCES} ${LIBRARY_FILESYSTEM_EMBEDDED_HEADERS})
target_link_libraries(library_filesystem_embedded PUBLIC library_filesystem_interface utility cmrc::base)
target_check_style(library_filesystem_embedded)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_FILESYSTEM_EMBEDDED_SOURCES Tests/EmbeddedFileSystem_ut.cpp)

    cmrc_add_resource_library(test_library_filesystem_embedded_resources ${TEST_LIBRARY_FILESYSTEM_EMBEDDED_SOURCES} NAMESPACE testrc)

    add_library(test_library_filesystem_embedded OBJECT ${TEST_LIBRARY_FILESYSTEM_EMBEDDED_SOURCES})
    target_link_libraries(test_library_filesystem_embedded PUBLIC testing_unit library_filesystem_embedded test_library_filesystem_embedded_resources)

    target_check_style(test_library_filesystem_embedded)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_filesystem_embedded)
endif()
