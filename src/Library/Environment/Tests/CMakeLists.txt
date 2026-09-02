cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_ENVIRONMENT_SOURCES
            Environment_ut.cpp)

    add_library(test_library_environment OBJECT ${TEST_LIBRARY_ENVIRONMENT_SOURCES})
    target_check_style(test_library_environment)
    target_link_libraries(test_library_environment PUBLIC testing_unit library_environment_implementation)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_environment)
endif()
