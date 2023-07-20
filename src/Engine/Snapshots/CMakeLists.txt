cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(ENGINE_SERIALIZATION_SOURCES
        CompositeSnapshots.cpp
        EntitySnapshots.cpp
        TableSerialization.cpp)

set(ENGINE_SERIALIZATION_HEADERS
        CommonSnapshots.h
        CompositeSnapshots.h
        EntitySnapshots.h
        SnapshotSerialization.h
        TableSerialization.h)

add_library(engine_serialization STATIC ${ENGINE_SERIALIZATION_SOURCES} ${ENGINE_SERIALIZATION_HEADERS})
target_link_libraries(engine_serialization PUBLIC engine library_binary)
target_check_style(engine_serialization)
