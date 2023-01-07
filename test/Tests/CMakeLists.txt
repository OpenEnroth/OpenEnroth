cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

if(ENABLE_TESTS)
    set(TESTS_SOURCES TestPartyCreationMenu.cpp)

    add_library(tests OBJECT ${TESTS_SOURCES})
    target_compile_definitions(tests PRIVATE TEST_GROUP=Tests)

    target_check_style(tests)

    target_link_libraries(Open_Enroth_GameTest tests)
endif()
