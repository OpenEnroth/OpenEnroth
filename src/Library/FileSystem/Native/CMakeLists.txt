cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_FILESYSTEM_NATIVE_SOURCES
        NativeFileSystem.cpp)

set(LIBRARY_FILESYSTEM_NATIVE_HEADERS
        NativeFileSystem.h)

add_library(library_filesystem_native STATIC ${LIBRARY_FILESYSTEM_NATIVE_SOURCES} ${LIBRARY_FILESYSTEM_NATIVE_HEADERS})
target_link_libraries(library_filesystem_native PUBLIC library_filesystem_interface utility)
target_check_style(library_filesystem_native)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_FILESYSTEM_NATIVE_SOURCES Tests/NativeFileSystem_ut.cpp)

    add_library(test_library_filesystem_native OBJECT ${TEST_LIBRARY_FILESYSTEM_NATIVE_SOURCES})
    target_link_libraries(test_library_filesystem_native PUBLIC testing_unit library_filesystem_native)

    target_check_style(test_library_filesystem_native)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_filesystem_native)
endif()
