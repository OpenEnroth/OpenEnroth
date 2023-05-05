cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(ENGINE_SERIALIZATION_SOURCES
        CompositeImages.cpp
        LegacyImages.cpp)

set(ENGINE_SERIALIZATION_HEADERS
        CommonImages.h
        CompositeImages.h
        LegacyImages.h
        MultiStageSerialization.h)

add_library(engine_serialization STATIC ${ENGINE_SERIALIZATION_SOURCES} ${ENGINE_SERIALIZATION_HEADERS})
target_link_libraries(engine_serialization engine library_binary)
target_check_style(engine_serialization)
