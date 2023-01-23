cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(RANDOM_SOURCES Random.cpp)

set(RANDOM_HEADERS Random.h
                   RandomEngine.h
                   MersenneTwisterRandomEngine.h
                   NonRandomEngine.h)

add_library(random STATIC ${RANDOM_SOURCES} ${RANDOM_HEADERS})
target_check_style(random)
