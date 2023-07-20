cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(LIBRARY_SNAPSHOTS_SOURCES)

set(LIBRARY_SNAPSHOTS_HEADERS
        CommonSnapshots.h
        SnapshotSerialization.h)

# No sources here, so it's an INTERFACE library. Also, I have no idea how the text aligns so well.
add_library(library_snapshots INTERFACE ${LIBRARY_SNAPSHOTS_SOURCES} ${LIBRARY_SNAPSHOTS_HEADERS})
target_check_style(library_snapshots)
target_link_libraries(library_snapshots INTERFACE utility)
