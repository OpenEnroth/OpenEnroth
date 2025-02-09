cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_LOGGER_SOURCES
        LogCategory.cpp
        LogEnums.cpp
        Logger.cpp
        LogSink.cpp
        DistLogSink.cpp
        StreamLogSink.cpp
        RotatingLogSink.cpp)

set(LIBRARY_LOGGER_HEADERS
        BufferLogSink.h
        LogCategory.h
        LogEnums.h
        Logger.h
        LogSink.h
        DistLogSink.h
        LogSource.h
        StreamLogSink.h
        LogEnumFunctions.h
        RotatingLogSink.h)

add_library(library_logger STATIC ${LIBRARY_LOGGER_SOURCES} ${LIBRARY_LOGGER_HEADERS})
target_link_libraries(library_logger PUBLIC library_serialization library_filesystem_interface utility PRIVATE spdlog::spdlog)
target_check_style(library_logger)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_LOGGER_SOURCES Tests/RotatingLogSink_ut.cpp)

    add_library(test_library_logger OBJECT ${TEST_LIBRARY_LOGGER_SOURCES})
    target_link_libraries(test_library_logger PUBLIC testing_unit library_logger)

    target_check_style(test_library_logger)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_logger)
endif()
