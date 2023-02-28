#include "Logger.h"

#include <cstdio>
#include <string>

PlatformLogger *Logger::BaseLogger() const {
    return baseLogger_;
}

void Logger::SetBaseLogger(PlatformLogger *baseLogger) {
    baseLogger_ = baseLogger;
}

void Logger::LogV(PlatformLogLevel logLevel, fmt::string_view fmt, fmt::format_args args) {
    std::string message = fmt::vformat(fmt, args);

    if (baseLogger_) {
        baseLogger_->log(APPLICATION_LOG, logLevel, message.c_str());
    } else {
        fprintf(stderr, "UNINITIALIZED LOGGER: %s\n", message.c_str());
    }
}
