cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(UTILITY_SOURCES
        DataPath.cpp
        Exception.cpp
        FileSystem.cpp
        Math/TrigLut.cpp
        Memory/Blob.cpp
        Streams/BlobInputStream.cpp
        Streams/BlobOutputStream.cpp
        Streams/FileInputStream.cpp
        Streams/FileOutputStream.cpp
        Streams/InputStream.cpp
        Streams/MemoryInputStream.cpp
        Streams/StringOutputStream.cpp
        Streams/TempFileOutputStream.cpp
        String.cpp
        System.cpp)

set(UTILITY_HEADERS
        DataPath.h
        Embedded.h
        Exception.h
        FileSystem.h
        Flags.h
        Format.h
        Types.h
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
        ScopeGuard.h
        Segment.h
        Streams/BlobInputStream.h
        Streams/BlobOutputStream.h
        Streams/FileInputStream.h
        Streams/FileOutputStream.h
        Streams/InputStream.h
        Streams/MemoryInputStream.h
        Streams/OutputStream.h
        Streams/StringOutputStream.h
        Streams/TempFileOutputStream.h
        String.h
        Unaligned.h
        System.h)

add_library(utility STATIC ${UTILITY_SOURCES} ${UTILITY_HEADERS})
target_check_style(utility)
target_link_libraries(utility
        PUBLIC
        fmt::fmt
        PRIVATE
        mio::mio)

if(OE_BUILD_TESTS)
    set(TEST_UTILITY_SOURCES
            Math/Tests/Float_ut.cpp
            Memory/Tests/Blob_ut.cpp
            Streams/Tests/FileOutputStream_ut.cpp
            Tests/IndexedArray_ut.cpp
            Tests/IndexedBitset_ut.cpp
            Tests/Segment_ut.cpp
            Tests/String_ut.cpp
            Tests/System_ut.cpp)

    add_library(test_utility OBJECT ${TEST_UTILITY_SOURCES})
    target_link_libraries(test_utility PUBLIC testing_unit utility)

    target_check_style(test_utility)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_utility)
endif()
