cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(JSON_SOURCES JsonExceptions.cpp)

set(JSON_HEADERS Json.h
                 JsonFwd.h
                 JsonExceptions.h)

add_library(json STATIC ${JSON_SOURCES} ${JSON_HEADERS})
target_link_libraries(json utility nlohmann_json serialization)
target_check_style(json)


if(ENABLE_TESTS)
    set(TEST_JSON_SOURCES Tests/Json_ut.cpp)

    add_library(test_json OBJECT ${TEST_JSON_SOURCES})
    target_compile_definitions(test_json PRIVATE TEST_GROUP=Json)
    target_link_libraries(test_json json)

    target_check_style(test_json)

    target_link_libraries(OpenEnroth_UnitTest test_json)
endif()
