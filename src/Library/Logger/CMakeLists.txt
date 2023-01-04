cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(LOGGER_SOURCES Logger.cpp)

set(LOGGER_HEADERS Logger.h)

add_library(logger STATIC ${LOGGER_SOURCES} ${LOGGER_HEADERS})
target_link_libraries(logger platform)
target_check_style(logger)
