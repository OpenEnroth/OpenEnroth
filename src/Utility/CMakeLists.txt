cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(UTILITY_SOURCES Blob.cpp
                    Color.cpp
                    DataPath.cpp
                    Log.cpp
                    String.cpp
                    Math/TrigLut.cpp
                    Streams/MemoryInputStream.cpp
                    Streams/StringOutputStream.cpp
                    Streams/FileOutputStream.cpp)

set(UTILITY_HEADERS Blob.h
                    Color.h
                    DataPath.h
                    Embedded.h
                    Flags.h
                    FreeDeleter.h
                    IndexedArray.h
                    Log.h
                    Memory.h
                    Segment.h
                    String.h
                    Geometry/BBox.h
                    Geometry/Plane.h
                    Geometry/Vec.h
                    Geometry/Size.h
                    Geometry/Point.h
                    Workaround/ToUnderlying.h
                    Math/Float.h
                    Math/TrigLut.h
                    Streams/InputStream.h
                    Streams/OutputStream.h
                    Streams/MemoryInputStream.h
                    Streams/StringOutputStream.h
                    Streams/FileOutputStream.h)

add_library(utility STATIC ${UTILITY_SOURCES} ${UTILITY_HEADERS})
target_check_style(utility)

if(ENABLE_TESTS)
    set(TEST_UTILITY_SOURCES Tests/IndexedArray_ut.cpp
                             Tests/Segment_ut.cpp
                             Math/Tests/Float_ut.cpp
                             Streams/Tests/FileOutputStream_ut.cpp)

    add_library(test_utility OBJECT ${TEST_UTILITY_SOURCES})
    target_compile_definitions(test_utility PRIVATE TESTGROUP=Utility)
    target_link_libraries(test_utility utility)

    target_check_style(test_utility)

    target_link_libraries(World_of_Might_and_Magic_test test_utility)
endif()
