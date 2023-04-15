cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

if(ENABLE_TESTS)
    set(GAME_TEST_MAIN_SOURCES
            GameTestMain.cpp
            GameTestOptions.cpp)
    set(GAME_TEST_MAIN_HEADERS
            GameTestOptions.h)

    add_executable(OpenEnroth_GameTest ${GAME_TEST_MAIN_SOURCES} ${GAME_TEST_MAIN_HEADERS})
    target_fix_libcxx_assertions(OpenEnroth_GameTest)
    target_link_libraries(OpenEnroth_GameTest application testing_game GTest::gtest)
    target_compile_definitions(OpenEnroth_GameTest PRIVATE TEST_GROUP=None)

    target_check_style(OpenEnroth_GameTest)
    PREBUILT_DEPENDENCIES_RESOLVE(OpenEnroth_GameTest)


    # OpenEnroth_TestData
    ExternalProject_Add(OpenEnroth_TestData
            PREFIX ${CMAKE_CURRENT_BINARY_DIR}/test_data_tmp
            GIT_REPOSITORY https://github.com/OpenEnroth/OpenEnroth_TestData.git
            GIT_TAG 61568c08768f2998d376ed3c370451aeeaf9090e
            SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/test_data
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND "")


    # GameTest
    add_custom_target(GameTest
            OpenEnroth_GameTest --test-path ${CMAKE_CURRENT_BINARY_DIR}/test_data/data
            DEPENDS OpenEnroth_GameTest OpenEnroth_TestData
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()
