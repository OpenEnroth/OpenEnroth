cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(UTILITY_SOURCES
        Exception.cpp
        Math/TrigLut.cpp
        Memory/Blob.cpp
        Streams/BlobInputStream.cpp
        Streams/BlobOutputStream.cpp
        Streams/FileInputStream.cpp
        Streams/FileOutputStream.cpp
        Streams/InputStream.cpp
        Streams/OutputStream.cpp
        String/Ascii.cpp
        String/Encoding.cpp
        String/Split.cpp
        String/Transformations.cpp
        UnicodeCrt.cpp
        String/Wrap.cpp)

set(UTILITY_HEADERS
        Exception.h
        Flags.h
        Hash.h
        String/Format.h
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
        Streams/BlobInputStream.h
        Streams/BlobOutputStream.h
        Streams/FileInputStream.h
        Streams/FileOutputStream.h
        Streams/InputStream.h
        Streams/MemoryInputStream.h
        Streams/OutputStream.h
        Streams/StreamBuffer.h
        String/Ascii.h
        String/Encoding.h
        String/Join.h
        String/Split.h
        String/TransparentFunctors.h
        String/Transformations.h
        String/Unicode.h
        Unaligned.h
        UnicodeCrt.h
        SmallVector.h
        ScopedRollback.h
        String/Wrap.h)

add_library(utility STATIC ${UTILITY_SOURCES} ${UTILITY_HEADERS})
target_check_style(utility)
target_link_libraries(utility
        PUBLIC
        fmt::fmt
        gch::small_vector
        Boost::lambda2
        PRIVATE
        ztd::text
        mio::mio)

# Tweak ztd buffer size, default is 512Kb, but default stack size of non-main threads on MacOS is also 512Kb.
target_compile_definitions(utility PRIVATE ZTD_INTERMEDIATE_BUFFER_SUGGESTED_BYTE_SIZE=65536)

if(OE_BUILD_TESTS)
    set(TEST_UTILITY_SOURCES
            Math/Tests/Float_ut.cpp
            Memory/Tests/Blob_ut.cpp
            Streams/Tests/BlobInputStream_ut.cpp
            Streams/Tests/BlobOutputStream_ut.cpp
            Streams/Tests/FileOutputStream_ut.cpp
            Streams/Tests/FileInputStream_ut.cpp
            Streams/Tests/InputStream_ut.cpp
            Tests/IndexedArray_ut.cpp
            Tests/IndexedBitset_ut.cpp
            Tests/Segment_ut.cpp
            Tests/UnicodeCrt_ut.cpp
            String/Tests/Transformations_ut.cpp
            String/Tests/TransparentFunctors_ut.cpp
            String/Tests/Ascii_ut.cpp
            String/Tests/Encoding_ut.cpp
            String/Tests/Split_ut.cpp
            String/Tests/Join_ut.cpp
            String/Tests/Wrap_ut.cpp)

    add_library(test_utility OBJECT ${TEST_UTILITY_SOURCES})
    target_link_libraries(test_utility PUBLIC testing_unit utility)

    target_check_style(test_utility)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_utility)
endif()
