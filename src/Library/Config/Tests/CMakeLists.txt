cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_CONFIG_SOURCES
            Config_ut.cpp)

    add_library(test_library_config OBJECT ${TEST_LIBRARY_CONFIG_SOURCES})
    target_check_style(test_library_config)
    target_link_libraries(test_library_config PUBLIC testing_unit library_config library_logger)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_config)
endif()
