cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(SERIALIZATION_SOURCES SerializationExceptions.cpp
                          EnumSerializer.cpp
                          StandardSerialization.cpp)

set(SERIALIZATION_HEADERS Serialization.h
                          SerializationFwd.h
                          EnumSerialization.h
                          EnumSerializer.h
                          SerializationExceptions.h
                          StandardSerialization.h)

add_library(serialization STATIC ${SERIALIZATION_SOURCES} ${SERIALIZATION_HEADERS})
target_link_libraries(serialization utility magic_enum::magic_enum FastFloat::fast_float)
target_check_style(serialization)


if(ENABLE_TESTS)
    set(TEST_SERIALIZATION_SOURCES Tests/Serialization_ut.cpp)

    add_library(test_serialization OBJECT ${TEST_SERIALIZATION_SOURCES})
    target_compile_definitions(test_serialization PRIVATE TEST_GROUP=Serialization)
    target_link_libraries(test_serialization serialization)

    target_check_style(test_serialization)

    target_link_libraries(World_of_Might_and_Magic_UnitTest test_serialization)
endif()
