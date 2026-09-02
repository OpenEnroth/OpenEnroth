cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_FILESYSTEM_INTERFACE_SOURCES
        FileSystem.cpp
        FileSystemPath.cpp
        FileSystemException.cpp
        ReadOnlyFileSystem.cpp)

set(LIBRARY_FILESYSTEM_INTERFACE_HEADERS
        FileSystem.h
        FileSystemPath.h
        FileSystemEnums.h
        FileSystemException.h
        ReadOnlyFileSystem.h
        FileSystemFwd.h
        FileSystemPathView.h
        FileSystemPathSplit.h
        FileSystemPathComponents.h)

add_library(library_filesystem_interface STATIC ${LIBRARY_FILESYSTEM_INTERFACE_SOURCES} ${LIBRARY_FILESYSTEM_INTERFACE_HEADERS})
target_link_libraries(library_filesystem_interface PUBLIC utility)
target_check_style(library_filesystem_interface)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_FILESYSTEM_INTERFACE_SOURCES Tests/FileSystemPath_ut.cpp)

    add_library(test_library_filesystem_interface OBJECT ${TEST_LIBRARY_FILESYSTEM_INTERFACE_SOURCES})
    target_link_libraries(test_library_filesystem_interface PUBLIC testing_unit library_filesystem_interface)

    target_check_style(test_library_filesystem_interface)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_filesystem_interface)
endif()
