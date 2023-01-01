cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

if(ENABLE_TESTS)
    set(GAME_TEST_MAIN_SOURCES GameTestMain.cpp
                               GameTestOptions.cpp)
    set(GAME_TEST_MAIN_HEADERS GameTestOptions.h)

    add_executable(World_of_Might_and_Magic_GameTest ${GAME_TEST_MAIN_SOURCES} ${GAME_TEST_MAIN_HEADERS})
    target_fix_libcxx_assertions(World_of_Might_and_Magic_GameTest)
    target_link_libraries(World_of_Might_and_Magic_GameTest game game_test test_engine GTest::gtest)
    target_compile_definitions(World_of_Might_and_Magic_GameTest PRIVATE TEST_GROUP=None)

    target_check_style(World_of_Might_and_Magic_GameTest)
    PREBUILT_DEPENDENCIES_RESOLVE(World_of_Might_and_Magic_GameTest)


    # OpenEnroth_TestData
    ExternalProject_Add(OpenEnroth_TestData
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/test_data_tmp
        GIT_REPOSITORY https://github.com/OpenEnroth/OpenEnroth_TestData.git
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/test_data
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
    )


    # GameTest
    add_custom_target(GameTest
        World_of_Might_and_Magic_GameTest --test-data ${CMAKE_CURRENT_BINARY_DIR}/test_data/data
        DEPENDS World_of_Might_and_Magic_GameTest OpenEnroth_TestData
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()
