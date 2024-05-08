cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(FSM_SOURCES
        Fsm.cpp
        FsmAction.cpp
        FsmBuilder.cpp
        FsmState.cpp)

set(FSM_HEADERS
        Fsm.h
        FsmAction.h
        FsmBuilder.h
        FsmEventHandler.h
        FsmState.h
        FsmTypes.h)

add_library(library_fsm STATIC ${FSM_SOURCES} ${FSM_HEADERS})
target_link_libraries(library_fsm PUBLIC utility library_logger library_platform_filters)
target_check_style(library_fsm)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_FSM_SOURCES
            Tests/Fsm_ut.cpp)

    add_library(test_library_fsm OBJECT ${TEST_LIBRARY_FSM_SOURCES})
    target_link_libraries(test_library_fsm PUBLIC testing_unit utility library_fsm)

    target_check_style(test_library_fsm)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_fsm)
endif()
