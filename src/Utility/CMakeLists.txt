cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(UTILITY_SOURCES Color.cpp
                    DataPath.cpp
                    FileSystem.cpp
                    String.cpp
                    Memory/Blob.cpp
                    Math/TrigLut.cpp
                    Random/Random.cpp
                    Streams/MemoryInputStream.cpp
                    Streams/StringOutputStream.cpp
                    Streams/FileOutputStream.cpp)

set(UTILITY_HEADERS Color.h
                    DataPath.h
                    Embedded.h
                    FileSystem.h
                    Flags.h
                    Format.h
                    FreeDeleter.h
                    IndexedArray.h
                    ScopeGuard.h
                    Segment.h
                    String.h
                    Memory/Blob.h
                    Memory/MemSet.h
                    Geometry/BBox.h
                    Geometry/Margins.h
                    Geometry/Plane.h
                    Geometry/Vec.h
                    Geometry/Size.h
                    Geometry/Point.h
                    Math/Float.h
                    Math/TrigLut.h
                    Random/Random.h
                    Random/RandomEngine.h
                    Random/MersenneTwisterRandomEngine.h
                    Random/NonRandomEngine.h
                    Streams/InputStream.h
                    Streams/OutputStream.h
                    Streams/MemoryInputStream.h
                    Streams/StringOutputStream.h
                    Streams/FileOutputStream.h)

add_library(utility STATIC ${UTILITY_SOURCES} ${UTILITY_HEADERS})
target_link_libraries(utility fmt::fmt)
target_check_style(utility)

if(ENABLE_TESTS)
    set(TEST_UTILITY_SOURCES Tests/IndexedArray_ut.cpp
                             Tests/Segment_ut.cpp
                             Math/Tests/Float_ut.cpp
                             Streams/Tests/FileOutputStream_ut.cpp
                             Tests/String_ut.cpp)

    add_library(test_utility OBJECT ${TEST_UTILITY_SOURCES})
    target_compile_definitions(test_utility PRIVATE TEST_GROUP=Utility)
    target_link_libraries(test_utility utility)

    target_check_style(test_utility)

    target_link_libraries(Open_Enroth_UnitTest test_utility)
endif()
