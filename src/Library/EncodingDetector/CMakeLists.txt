cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_ENCODING_DETECTOR_SOURCES
        EncodingDetector.cpp
        EncodingEnums.cpp)

set(LIBRARY_ENCODING_DETECTOR_HEADERS
        EncodingDetector.h
        EncodingEnums.h)

add_library(library_encoding_detector STATIC ${LIBRARY_ENCODING_DETECTOR_SOURCES} ${LIBRARY_ENCODING_DETECTOR_HEADERS})
target_link_libraries(library_encoding_detector PUBLIC library_serialization PRIVATE libuchardet ztd::text)
target_check_style(library_encoding_detector)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_ENCODING_DETECTOR_SOURCES
            Tests/EncodingDetector_ut.cpp)

    add_library(test_library_encoding_detector OBJECT ${TEST_LIBRARY_ENCODING_DETECTOR_SOURCES})
    target_link_libraries(test_library_encoding_detector PUBLIC testing_unit library_encoding_detector)

    target_check_style(test_library_encoding_detector)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_encoding_detector)
endif()
