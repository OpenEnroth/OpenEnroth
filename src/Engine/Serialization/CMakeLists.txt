cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(ENGINE_SERIALIZATION_SOURCES
        LegacyImages.cpp)

set(ENGINE_SERIALIZATION_HEADERS
        Deserializer.h
        LegacyImages.h
        Serializer.h)

add_library(engine_serialization STATIC ${ENGINE_SERIALIZATION_SOURCES} ${ENGINE_SERIALIZATION_HEADERS})
target_link_libraries(engine_serialization engine)
target_check_style(engine_serialization)
