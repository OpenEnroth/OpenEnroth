cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_SERIALIZATION_SOURCES
        EnumSerializer.cpp
        SerializationExceptions.cpp
        StandardSerialization.cpp)

set(LIBRARY_SERIALIZATION_HEADERS
        EnumSerialization.h
        EnumSerializer.h
        Serialization.h
        SerializationExceptions.h
        SerializationFwd.h
        StandardSerialization.h)

add_library(library_serialization STATIC ${LIBRARY_SERIALIZATION_SOURCES} ${LIBRARY_SERIALIZATION_HEADERS})
target_check_style(library_serialization)
target_link_libraries(library_serialization
        PUBLIC
        utility
        magic_enum::magic_enum
        PRIVATE
        FastFloat::fast_float)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_SERIALIZATION_SOURCES Tests/Serialization_ut.cpp)

    add_library(test_library_serialization OBJECT ${TEST_LIBRARY_SERIALIZATION_SOURCES})
    target_link_libraries(test_library_serialization PUBLIC testing_unit library_serialization)

    target_check_style(test_library_serialization)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_serialization)
endif()
