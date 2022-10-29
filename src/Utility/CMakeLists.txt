cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(UTILITY_SOURCES Blob.cpp
                    Color.cpp
                    DataPath.cpp
                    String.cpp
                    Streams/MemoryInputStream.cpp
                    Streams/StringOutputStream.cpp
                    Streams/FileOutputStream.cpp)

set(UTILITY_TESTS   IndexedArray_ut.cpp)

set(UTILITY_HEADERS Blob.h
                    Color.h
                    DataPath.h
                    Embedded.h
                    Flags.h
                    FreeDeleter.h
                    IndexedArray.h
                    Memory.h
                    String.h
                    Workaround/ToUnderlying.h
                    Streams/InputStream.h
                    Streams/OutputStream.h
                    Streams/MemoryInputStream.h
                    Streams/StringOutputStream.h
                    Streams/FileOutputStream.h)

add_library(utility STATIC ${UTILITY_SOURCES} ${UTILITY_HEADERS})
target_check_style(utility)

add_library(utility_test OBJECT ${UTILITY_TESTS} ${UTILITY_HEADERS})
target_link_libraries(utility_test utility)
target_check_style(utility_test)
