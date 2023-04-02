#include "Logger.h"

#include <cstdio>
#include <string>

PlatformLogger *Logger::baseLogger() const {
    return _baseLogger;
}

void Logger::setBaseLogger(PlatformLogger *baseLogger) {
    _baseLogger = baseLogger;
}

void Logger::logV(PlatformLogLevel logLevel, fmt::string_view fmt, fmt::format_args args) {
    std::string message = fmt::vformat(fmt, args);

    if (_baseLogger) {
        _baseLogger->log(APPLICATION_LOG, logLevel, message.c_str());
    } else {
        fprintf(stderr, "UNINITIALIZED LOGGER: %s\n", message.c_str());
    }
}
