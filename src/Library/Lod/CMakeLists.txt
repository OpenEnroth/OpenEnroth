cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(LIBRARY_LOD_SOURCES
        LodReader.cpp
        LodEnums.cpp
        LodSnapshots.cpp
        LodWriter.cpp)

set(LIBRARY_LOD_HEADERS
        LodReader.h
        LodEnums.h
        LodInfo.h
        LodSnapshots.h
        LodWriter.h)

add_library(library_lod STATIC ${LIBRARY_LOD_SOURCES} ${LIBRARY_LOD_HEADERS})
target_link_libraries(library_lod PUBLIC library_serialization library_binary library_snapshots utility)
target_check_style(library_lod)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_LOD_SOURCES
            Tests/LodReader_ut.cpp
            Tests/LodWriter_ut.cpp)

    add_library(test_library_lod OBJECT ${TEST_LIBRARY_LOD_SOURCES})
    target_link_libraries(test_library_lod PUBLIC testing_unit library_lod)

    target_check_style(test_library_lod)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_lod)
endif()
