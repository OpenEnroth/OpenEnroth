cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(CORE_SERIALIZATION_SOURCES
        PlatformKeySerialization.cpp
        PlatformWindowModeSerialization.cpp)

set(CORE_SERIALIZATION_HEADERS
        PlatformKeySerialization.h
        PlatformWindowModeSerialization.h)

add_library(core_serialization STATIC ${CORE_SERIALIZATION_SOURCES} ${CORE_SERIALIZATION_HEADERS})
target_check_cpp_style(core_serialization)
target_link_libraries(core_serialization PUBLIC
        library_serialization
        library_platform_interface)
