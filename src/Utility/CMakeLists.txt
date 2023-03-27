cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(UTILITY_SOURCES
        Color.cpp
        DataPath.cpp
        Exception.cpp
        FileSystem.cpp
        Math/TrigLut.cpp
        Memory/Blob.cpp
        Streams/FileInputStream.cpp
        Streams/FileOutputStream.cpp
        Streams/InputStream.cpp
        Streams/MemoryInputStream.cpp
        Streams/StringOutputStream.cpp
        String.cpp)

set(UTILITY_HEADERS
        Color.h
        DataPath.h
        Embedded.h
        Exception.h
        FileSystem.h
        Flags.h
        Format.h
        Geometry/BBox.h
        Geometry/Margins.h
        Geometry/Plane.h
        Geometry/Point.h
        Geometry/Size.h
        Geometry/Vec.h
        IndexedArray.h
        Math/Float.h
        Math/TrigLut.h
        Memory/Blob.h
        Memory/FreeDeleter.h
        Memory/MemSet.h
        Reversed.h
        ScopeGuard.h
        Segment.h
        Streams/FileInputStream.h
        Streams/FileOutputStream.h
        Streams/InputStream.h
        Streams/MemoryInputStream.h
        Streams/OutputStream.h
        Streams/StringOutputStream.h
        String.h)

add_library(utility STATIC ${UTILITY_SOURCES} ${UTILITY_HEADERS})
target_link_libraries(utility fmt::fmt mio::mio)
target_check_style(utility)

if(ENABLE_TESTS)
    set(TEST_UTILITY_SOURCES
            Math/Tests/Float_ut.cpp
            Streams/Tests/FileOutputStream_ut.cpp
            Tests/IndexedArray_ut.cpp
            Tests/Segment_ut.cpp
            Tests/String_ut.cpp)

    add_library(test_utility OBJECT ${TEST_UTILITY_SOURCES})
    target_compile_definitions(test_utility PRIVATE TEST_GROUP=Utility)
    target_link_libraries(test_utility utility)

    target_check_style(test_utility)

    target_link_libraries(OpenEnroth_UnitTest test_utility)
endif()
