cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(ENGINE_SERIALIZATION_SOURCES
        CompositeSnapshots.cpp
        EntitySnapshots.cpp
        EnumSnapshots.cpp
        LegacyEncodingDetector.cpp
        TableSerialization.cpp)

set(ENGINE_SERIALIZATION_HEADERS
        CompositeSnapshots.h
        EntitySnapshots.h
        EnumSnapshots.h
        LegacyEncodingDetector.h
        TableSerialization.h)

add_library(engine_snapshots STATIC ${ENGINE_SERIALIZATION_SOURCES} ${ENGINE_SERIALIZATION_HEADERS})
target_link_libraries(engine_snapshots PUBLIC engine library_binary library_snapshots library_json)
target_check_style(engine_snapshots)

if(OE_BUILD_TESTS)
    set(TEST_ENGINE_SNAPSHOTS_SOURCES
            Tests/LegacyEncodingDetector_ut.cpp)

    add_library(test_engine_snapshots OBJECT ${TEST_ENGINE_SNAPSHOTS_SOURCES})
    target_link_libraries(test_engine_snapshots PUBLIC testing_unit engine_snapshots)

    target_check_style(test_engine_snapshots)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_engine_snapshots)
endif()
