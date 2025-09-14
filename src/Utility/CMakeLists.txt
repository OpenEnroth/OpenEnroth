cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(UTILITY_SOURCES
        Exception.cpp
        Math/TrigLut.cpp
        Memory/Blob.cpp
        SequentialBlobReader.cpp
        Streams/BlobInputStream.cpp
        Streams/BlobOutputStream.cpp
        Streams/FileInputStream.cpp
        Streams/FileOutputStream.cpp
        Streams/InputStream.cpp
        Streams/MemoryInputStream.cpp
        Streams/StringOutputStream.cpp
        String/Ascii.cpp
        String/Split.cpp
        String/Transformations.cpp
        UnicodeCrt.cpp
        String/Wrap.cpp)

set(UTILITY_HEADERS
        Embedded.h
        Exception.h
        Flags.h
        Hash.h
        String/Format.h
        Types.h
        IndexedArray.h
        IndexedBitset.h
        Lambda.h
        Math/Float.h
        Math/TrigLut.h
        Memory/Blob.h
        Memory/FreeDeleter.h
        Memory/MemSet.h
        ScopeGuard.h
        Segment.h
        SequentialBlobReader.h
        Streams/BlobInputStream.h
        Streams/BlobOutputStream.h
        Streams/FileInputStream.h
        Streams/FileOutputStream.h
        Streams/InputStream.h
        Streams/MemoryInputStream.h
        Streams/OutputStream.h
        Streams/StringOutputStream.h
        String/Ascii.h
        String/Join.h
        String/Split.h
        String/TransparentFunctors.h
        String/Transformations.h
        Unaligned.h
        UnicodeCrt.h
        SmallVector.h
        ScopedRollback.h
        String/Wrap.h)

if(OE_BUILD_PLATFORM STREQUAL "windows")
    list(APPEND UTILITY_HEADERS Win/Unicode.h)
    list(APPEND UTILITY_SOURCES Win/Unicode.cpp)
endif()

add_library(utility STATIC ${UTILITY_SOURCES} ${UTILITY_HEADERS})
target_check_style(utility)
target_link_libraries(utility
        PUBLIC
        fmt::fmt
        gch::small_vector
        Boost::lambda2
        PRIVATE
        mio::mio)

if(OE_BUILD_TESTS)
    set(TEST_UTILITY_SOURCES
            Math/Tests/Float_ut.cpp
            Memory/Tests/Blob_ut.cpp
            Streams/Tests/FileOutputStream_ut.cpp
            Streams/Tests/FileInputStream_ut.cpp
            Streams/Tests/InputStream_ut.cpp
            Streams/Tests/MemoryInputStream_ut.cpp
            Tests/IndexedArray_ut.cpp
            Tests/IndexedBitset_ut.cpp
            Tests/Segment_ut.cpp
            Tests/UnicodeCrt_ut.cpp
            String/Tests/Transformations_ut.cpp
            String/Tests/TransparentFunctors_ut.cpp
            String/Tests/Ascii_ut.cpp
            String/Tests/Split_ut.cpp
            String/Tests/Join_ut.cpp
            String/Tests/Wrap_ut.cpp)

    if(OE_BUILD_PLATFORM STREQUAL "windows")
        list(APPEND TEST_UTILITY_SOURCES Win/Tests/Unicode_ut.cpp)
    endif()

    add_library(test_utility OBJECT ${TEST_UTILITY_SOURCES})
    target_link_libraries(test_utility PUBLIC testing_unit utility)

    target_check_style(test_utility)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_utility)
endif()
