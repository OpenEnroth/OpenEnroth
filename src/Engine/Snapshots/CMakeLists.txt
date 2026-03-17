cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(ENGINE_SERIALIZATION_SOURCES
        CompositeSnapshots.cpp
        EntitySnapshots.cpp
        EnumSnapshots.cpp
        TableSerialization.cpp)

set(ENGINE_SERIALIZATION_HEADERS
        CompositeSnapshots.h
        EntitySnapshots.h
        EnumSnapshots.h
        TableSerialization.h)

add_library(engine_snapshots STATIC ${ENGINE_SERIALIZATION_SOURCES} ${ENGINE_SERIALIZATION_HEADERS})
target_link_libraries(engine_snapshots PUBLIC engine library_binary library_snapshots library_encoding_detector library_json)
target_check_style(engine_snapshots)
