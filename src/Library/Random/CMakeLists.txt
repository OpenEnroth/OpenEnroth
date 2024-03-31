cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_RANDOM_SOURCES
        RandomEngine.cpp)

set(LIBRARY_RANDOM_HEADERS
        MersenneTwisterRandomEngine.h
        SequentialRandomEngine.h
        RandomEngine.h)

add_library(library_random STATIC ${LIBRARY_RANDOM_SOURCES} ${LIBRARY_RANDOM_HEADERS})
target_check_style(library_random)
