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
    if (baseLogger_ && baseLogger_->logLevel(ApplicationLog) > logLevel)
        return;

    va_list args;
    va_start(args, format);
    LogV(logLevel, format, args);
    va_end(args);
}

void Logger::Info(const char *pFormat, ...) {
    if (baseLogger_ && baseLogger_->logLevel(ApplicationLog) > LogInfo)
        return;

    va_list args;
    va_start(args, pFormat);
    LogV(LogInfo, pFormat, args);
    va_end(args);
}

void Logger::Warning(const char *pFormat, ...) {
    if (baseLogger_ && baseLogger_->logLevel(ApplicationLog) > LogWarning)
        return;

    va_list args;
    va_start(args, pFormat);
    LogV(LogWarning, pFormat, args);
    va_end(args);
}

void Logger::LogV(PlatformLogLevel logLevel, const char *pFormat, va_list args) {
    char message[8192];
    vsnprintf(message, 8192, pFormat, args);

    if (baseLogger_) {
        baseLogger_->log(ApplicationLog, logLevel, message);
    } else {
        fprintf(stderr, "UNINITIALIZED LOGGER: %s\n", message);
    }
}
