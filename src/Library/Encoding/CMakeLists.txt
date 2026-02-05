cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_ENCODING_SOURCES
        EncodingDetector.cpp
        EncodingEnums.cpp
        Encoding.cpp)

set(LIBRARY_ENCODING_HEADERS
        EncodingDetector.h
        EncodingEnums.h
        Encoding.h)

add_library(library_encoding STATIC ${LIBRARY_ENCODING_SOURCES} ${LIBRARY_ENCODING_HEADERS})
target_link_libraries(library_encoding PUBLIC library_serialization PRIVATE libuchardet ztd::text)
target_check_style(library_encoding)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_ENCODING_SOURCES
            Tests/EncodingDetector_ut.cpp
            Tests/Encoding_ut.cpp)

    add_library(test_library_encoding OBJECT ${TEST_LIBRARY_ENCODING_SOURCES})
    target_link_libraries(test_library_encoding PUBLIC testing_unit library_encoding)

    target_check_style(test_library_encoding)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_encoding)
endif()
