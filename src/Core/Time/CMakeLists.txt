cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(CORE_TIME_SOURCES
        Duration.cpp)

set(CORE_TIME_HEADERS
        Duration.h
        Time.h)

add_library(core_time STATIC ${CORE_TIME_SOURCES} ${CORE_TIME_HEADERS})
target_check_cpp_style(core_time)
target_link_libraries(core_time PUBLIC
        library_random)
