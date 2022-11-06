cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(UTILITY_SOURCES Blob.cpp
                    Color.cpp
                    DataPath.cpp
                    String.cpp
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
                    Memory.h
                    Segment.h
                    String.h
                    Workaround/ToUnderlying.h
                    Streams/InputStream.h
                    Streams/OutputStream.h
                    Streams/MemoryInputStream.h
                    Streams/StringOutputStream.h
                    Streams/FileOutputStream.h)

add_library(utility STATIC ${UTILITY_SOURCES} ${UTILITY_HEADERS})
target_check_style(utility)

if(ENABLE_TESTS)
    add_subdirectory(Tests)
    add_subdirectory(Streams/Tests)
endif()
