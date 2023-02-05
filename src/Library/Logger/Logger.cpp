#include <cstdarg>
#include <cstdio>

#include "Logger.h"

PlatformLogger *Logger::BaseLogger() const {
    return baseLogger_;
}

void Logger::SetBaseLogger(PlatformLogger *baseLogger) {
    baseLogger_ = baseLogger;
}

void Logger::Log(PlatformLogLevel logLevel, const char *format, ...) {
    if (baseLogger_ && baseLogger_->logLevel(APPLICATION_LOG) > logLevel)
        return;

    va_list args;
    va_start(args, format);
    LogV(logLevel, format, args);
    va_end(args);
}

void Logger::Info(const char *pFormat, ...) {
    if (baseLogger_ && baseLogger_->logLevel(APPLICATION_LOG) > LOG_INFO)
        return;

    va_list args;
    va_start(args, pFormat);
    LogV(LOG_INFO, pFormat, args);
    va_end(args);
}

void Logger::Warning(const char *pFormat, ...) {
    if (baseLogger_ && baseLogger_->logLevel(APPLICATION_LOG) > LOG_WARNING)
        return;

    va_list args;
    va_start(args, pFormat);
    LogV(LOG_WARNING, pFormat, args);
    va_end(args);
}

void Logger::LogV(PlatformLogLevel logLevel, const char *pFormat, va_list args) {
    char message[8192];
    vsnprintf(message, 8192, pFormat, args);

    if (baseLogger_) {
        baseLogger_->log(APPLICATION_LOG, logLevel, message);
    } else {
        fprintf(stderr, "UNINITIALIZED LOGGER: %s\n", message);
    }
}
