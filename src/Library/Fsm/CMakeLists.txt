cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(FSM_SOURCES
        FSM.cpp
        FSMBuilder.cpp
        FSMState.cpp)

set(FSM_HEADERS
        FSM.h
        FSMBuilder.h
        FSMEventHandler.h
        FSMState.h
        FSMTransitionHandler.h)

add_library(library_fsm STATIC ${FSM_SOURCES} ${FSM_HEADERS})
target_link_libraries(library_fsm PUBLIC utility library_logger)
target_check_style(library_fsm)
