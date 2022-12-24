cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(LOG_SOURCES Logger.cpp)

set(LOG_HEADERS Logger.h)

add_library(log STATIC ${LOG_SOURCES} ${LOG_HEADERS})
target_link_libraries(log platform)
target_check_style(log)
