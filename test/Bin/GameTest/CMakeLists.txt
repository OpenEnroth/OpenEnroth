cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

if(ENABLE_TESTS)
    set(GAME_TEST_MAIN_SOURCES GameTestMain.cpp)

    add_executable(World_of_Might_and_Magic_GameTest ${GAME_TEST_MAIN_SOURCES})
    target_fix_libcxx_assertions(World_of_Might_and_Magic_GameTest)
    target_link_libraries(World_of_Might_and_Magic_GameTest game game_test test_engine GTest::gtest)
    target_compile_definitions(World_of_Might_and_Magic_GameTest PRIVATE TEST_GROUP=None)

    target_check_style(World_of_Might_and_Magic_GameTest)
    PREBUILT_DEPENDENCIES_RESOLVE(World_of_Might_and_Magic_GameTest)
endif()
