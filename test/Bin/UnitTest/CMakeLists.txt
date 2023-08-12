cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

if(OE_BUILD_TESTS)
    set(UNIT_TEST_MAIN_SOURCES
            UnitTestMain.cpp)

    add_executable(OpenEnroth_UnitTest ${UNIT_TEST_MAIN_SOURCES})
    target_fix_libcxx_assertions(OpenEnroth_UnitTest)
    target_link_libraries(OpenEnroth_UnitTest PUBLIC testing_unit CLI11::CLI11)

    add_custom_target(UnitTest OpenEnroth_UnitTest
            DEPENDS OpenEnroth_UnitTest
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            USES_TERMINAL)

    target_check_style(OpenEnroth_UnitTest)
endif()
