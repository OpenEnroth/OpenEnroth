cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

if(ENABLE_TESTS)
    set(UNIT_TEST_MAIN_SOURCES UnitTestMain.cpp)

    add_executable(Open_Enroth_UnitTest ${UNIT_TEST_MAIN_SOURCES})
    target_fix_libcxx_assertions(Open_Enroth_UnitTest)
    target_link_libraries(Open_Enroth_UnitTest unit_test)

    add_custom_target(UnitTest Open_Enroth_UnitTest
        DEPENDS Open_Enroth_UnitTest
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

    target_check_style(Open_Enroth_UnitTest)
endif()
