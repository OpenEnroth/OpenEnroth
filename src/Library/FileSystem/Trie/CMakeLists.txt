cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_FILESYSTEM_TRIE_SOURCES)

set(LIBRARY_FILESYSTEM_TRIE_HEADERS
        FileSystemTrie.h)

add_library(library_filesystem_trie INTERFACE ${LIBRARY_FILESYSTEM_TRIE_SOURCES} ${LIBRARY_FILESYSTEM_TRIE_HEADERS})
target_link_libraries(library_filesystem_trie INTERFACE library_filesystem_interface utility)
target_check_style(library_filesystem_trie)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_FILESYSTEM_TRIE_SOURCES Tests/FileSystemTrie_ut.cpp)

    add_library(test_library_filesystem_trie OBJECT ${TEST_LIBRARY_FILESYSTEM_TRIE_SOURCES})
    target_link_libraries(test_library_filesystem_trie PUBLIC testing_unit library_filesystem_trie)

    target_check_style(test_library_filesystem_trie)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_filesystem_trie)
endif()
