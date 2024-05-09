cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

if(OE_BUILD_TESTS)
    set(GAME_TEST_MAIN_SOURCES
            GameTestMain.cpp
            GameTestOptions.cpp
            GameTests_0000.cpp
            GameTests_0500.cpp
            GameTests_1000.cpp
            GameTests_1500.cpp)
    set(GAME_TEST_MAIN_HEADERS
            GameTestOptions.h)

    add_executable(OpenEnroth_GameTest ${GAME_TEST_MAIN_SOURCES} ${GAME_TEST_MAIN_HEADERS})
    target_link_libraries(OpenEnroth_GameTest PUBLIC application testing_game library_cli library_platform_main library_stack_trace)

    target_check_style(OpenEnroth_GameTest)


    # OpenEnroth_TestData
    ExternalProject_Add(OpenEnroth_TestData
            PREFIX ${CMAKE_CURRENT_BINARY_DIR}/test_data_tmp
            GIT_REPOSITORY https://github.com/OpenEnroth/OpenEnroth_TestData.git
            GIT_TAG bd542b7e6b8df5b8e51b0a4f303c6a72b4c2cc5b
            SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/test_data
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND "")


    # GameTest
    add_custom_target(GameTest
            OpenEnroth_GameTest --test-path ${CMAKE_CURRENT_BINARY_DIR}/test_data/data
            DEPENDS OpenEnroth_GameTest OpenEnroth_TestData
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            USES_TERMINAL) # USES_TERMINAL makes the command print progress as it goes.

    add_custom_target(GameTest_Headless
            OpenEnroth_GameTest --test-path ${CMAKE_CURRENT_BINARY_DIR}/test_data/data --headless
            DEPENDS OpenEnroth_GameTest OpenEnroth_TestData
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            USES_TERMINAL)

    # RetraceTest
    add_custom_target(RetraceTest
            OpenEnroth retrace --check-canonical --glob ${CMAKE_CURRENT_BINARY_DIR}/test_data/data/*.json
            DEPENDS OpenEnroth OpenEnroth_TestData
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            USES_TERMINAL
    )

    add_custom_target(RetraceTest_Headless
            OpenEnroth retrace --headless --check-canonical --glob ${CMAKE_CURRENT_BINARY_DIR}/test_data/data/*.json
            DEPENDS OpenEnroth OpenEnroth_TestData
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            USES_TERMINAL
    )

endif()
