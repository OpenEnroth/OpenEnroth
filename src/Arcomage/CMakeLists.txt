cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(ACROMAGE_SOURCES
        Arcomage.cpp
        ArcomageCards.cpp)

set(ACROMAGE_HEADERS
        Arcomage.h)

add_library(arcomage STATIC ${ACROMAGE_SOURCES} ${ACROMAGE_HEADERS})
target_link_libraries(arcomage PUBLIC utility engine gui media library_color)

target_check_style(arcomage)
