cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_JSON_SOURCES
        JsonExceptions.cpp)

set(LIBRARY_JSON_HEADERS
        Json.h
        JsonExceptions.h
        JsonFwd.h)

add_library(library_json STATIC ${LIBRARY_JSON_SOURCES} ${LIBRARY_JSON_HEADERS})
target_check_style(library_json)
target_link_libraries(library_json
        PUBLIC
        utility
        library_serialization
        nlohmann_json)


if(OE_BUILD_TESTS)
    set(TEST_LIBARY_JSON_SOURCES Tests/Json_ut.cpp)

    add_library(test_library_json OBJECT ${TEST_LIBARY_JSON_SOURCES})
    target_link_libraries(test_library_json PUBLIC testing_unit library_json)

    target_check_style(test_library_json)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_json)
endif()
