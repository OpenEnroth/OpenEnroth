cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(UTILITY_SOURCES Color.cpp
                    DataPath.cpp
                    MemBuffer.cpp
                    String.cpp)

set(UTILITY_HEADERS Color.h
                    DataPath.h
                    Flags.h
                    FreeDeleter.h
                    IndexedArray.h
                    MemBuffer.h
                    String.h
                    Workaround.h)

add_library(utility STATIC ${UTILITY_SOURCES} ${UTILITY_HEADERS})
target_check_style(utility)
