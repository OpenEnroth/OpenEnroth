cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_SNAPSHOTS_SOURCES)

set(LIBRARY_SNAPSHOTS_HEADERS
        CommonSnapshots.h
        SnapshotSerialization.h
        SnapshotConcepts.h)

# No sources here, so it's an INTERFACE library. Also, I have no idea how the text aligns so well.
add_library(library_snapshots INTERFACE ${LIBRARY_SNAPSHOTS_SOURCES} ${LIBRARY_SNAPSHOTS_HEADERS})
target_check_style(library_snapshots)
target_link_libraries(library_snapshots INTERFACE library_binary utility)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_SNAPSHOTS_SOURCES Tests/Snapshots_ut.cpp)

    add_library(test_library_snapshots OBJECT ${TEST_LIBRARY_SNAPSHOTS_SOURCES})
    target_link_libraries(test_library_snapshots PUBLIC testing_unit library_snapshots)

    target_check_style(test_library_snapshots)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_snapshots)
endif()
