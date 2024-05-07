cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(Fsm_SOURCES
        Fsm.cpp
        FsmAction.cpp
        FsmBuilder.cpp
        FsmState.cpp)

set(Fsm_HEADERS
        Fsm.h
        FsmAction.h
        FsmBuilder.h
        FsmEventHandler.h
        FsmState.h
        FsmTypes.h)

add_library(library_fsm STATIC ${Fsm_SOURCES} ${Fsm_HEADERS})
target_link_libraries(library_fsm PUBLIC utility library_logger)
target_check_style(library_fsm)
