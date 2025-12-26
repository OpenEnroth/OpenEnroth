cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_PREPROCESSOR_SOURCES
        Preprocessor.cpp)

set(LIBRARY_PREPROCESSOR_HEADERS
        Preprocessor.h)

add_library(library_preprocessor STATIC ${LIBRARY_PREPROCESSOR_SOURCES} ${LIBRARY_PREPROCESSOR_HEADERS})
target_link_libraries(library_preprocessor PUBLIC library_filesystem_interface)
target_link_libraries(library_preprocessor PRIVATE tcpp::tcpp)
target_check_style(library_preprocessor)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_PREPROCESSOR_SOURCES Tests/Preprocessor_ut.cpp)

    add_library(test_library_preprocessor OBJECT ${TEST_LIBRARY_PREPROCESSOR_SOURCES})
    target_link_libraries(test_library_preprocessor PUBLIC testing_unit library_preprocessor library_filesystem_memory)
    target_check_style(test_library_preprocessor)
    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_preprocessor)
endif()
