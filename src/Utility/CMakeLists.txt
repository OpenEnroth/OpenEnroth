cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(UTILITY_SOURCES Blob.cpp
                    Color.cpp
                    DataPath.cpp
                    String.cpp)

set(UTILITY_HEADERS Blob.h
                    Color.h
                    DataPath.h
                    Flags.h
                    FreeDeleter.h
                    IndexedArray.h
                    Memory.h
                    String.h
                    Workaround.h Blob.cpp)

add_library(utility STATIC ${UTILITY_SOURCES} ${UTILITY_HEADERS})
target_check_style(utility)
