cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(FSM_SOURCES
        FSM.cpp)

set(FSM_HEADERS
        FSM.h)

add_library(library_fsm STATIC ${FSM_SOURCES} ${FSM_HEADERS})
target_link_libraries(library_fsm PUBLIC utility library_logger)
target_check_style(library_fsm)
