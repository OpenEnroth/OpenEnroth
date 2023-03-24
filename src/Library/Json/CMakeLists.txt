cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(LIBRARY_JSON_SOURCES
        JsonExceptions.cpp)

set(LIBRARY_JSON_HEADERS
        Json.h
        JsonExceptions.h
        JsonFwd.h)

add_library(library_json STATIC ${LIBRARY_JSON_SOURCES} ${LIBRARY_JSON_HEADERS})
target_link_libraries(library_json utility nlohmann_json library_serialization)
target_check_style(library_json)


if(ENABLE_TESTS)
    set(TEST_LIBARY_JSON_SOURCES Tests/Json_ut.cpp)

    add_library(test_library_json OBJECT ${TEST_LIBARY_JSON_SOURCES})
    target_compile_definitions(test_library_json PRIVATE TEST_GROUP=Json)
    target_link_libraries(test_library_json library_json)

    target_check_style(test_library_json)

    target_link_libraries(OpenEnroth_UnitTest test_library_json)
endif()
