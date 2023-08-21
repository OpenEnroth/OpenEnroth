cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(LIBRARY_RANDOM_SOURCES
        Random.cpp
        RandomEngine.cpp)

set(LIBRARY_RANDOM_HEADERS
        MersenneTwisterRandomEngine.h
        SequentialRandomEngine.h
        Random.h
        RandomEngine.h
        RandomEngineEnums.h)

add_library(library_random STATIC ${LIBRARY_RANDOM_SOURCES} ${LIBRARY_RANDOM_HEADERS})
target_link_libraries(library_random utility backward)
target_check_style(library_random)
