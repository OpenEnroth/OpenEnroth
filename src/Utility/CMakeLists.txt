cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(UTILITY_SOURCES DataPath.cpp
                    MemBuffer.cpp)

set(UTILITY_HEADERS DataPath.h
                    Flags.h
                    FreeDeleter.h
                    MemBuffer.h
                    Workaround.h)

add_library(utility STATIC ${UTILITY_SOURCES} ${UTILITY_HEADERS})
target_check_style(utility)
