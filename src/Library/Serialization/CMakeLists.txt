cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

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
target_link_libraries(library_serialization utility magic_enum::magic_enum FastFloat::fast_float)
target_check_style(library_serialization)


if(ENABLE_TESTS)
    set(TEST_LIBRARY_SERIALIZATION_SOURCES Tests/Serialization_ut.cpp)

    add_library(test_library_serialization OBJECT ${TEST_LIBRARY_SERIALIZATION_SOURCES})
    target_compile_definitions(test_library_serialization PRIVATE TEST_GROUP=Serialization)
    target_link_libraries(test_library_serialization library_serialization)

    target_check_style(test_library_serialization)

    target_link_libraries(OpenEnroth_UnitTest test_library_serialization)
endif()
