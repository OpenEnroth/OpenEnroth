cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_LOGGER_SOURCES
        LogCategory.cpp
        LogEnums.cpp
        Logger.cpp
        LogSink.cpp
        LogSinkComposite.cpp)

set(LIBRARY_LOGGER_HEADERS
        BufferLogSink.h
        LogCategory.h
        LogEnums.h
        Logger.h
        LogSink.h
        LogSinkComposite.h
        LogSource.h)

add_library(library_logger STATIC ${LIBRARY_LOGGER_SOURCES} ${LIBRARY_LOGGER_HEADERS})
target_link_libraries(library_logger PUBLIC library_serialization utility PRIVATE spdlog::spdlog)
target_check_style(library_logger)
