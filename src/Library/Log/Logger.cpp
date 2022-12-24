#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <cassert>

#include "Logger.h"

PlatformLogger *Logger::BaseLogger() const {
    return baseLogger_;
}

void Logger::SetBaseLogger(PlatformLogger *baseLogger) {
    baseLogger_ = baseLogger;
}

void Logger::Write(PlatformLogLevel logLevel, const char *format, ...) {
    if (baseLogger_ && baseLogger_->LogLevel(ApplicationLog) > logLevel)
        return;

    va_list args;
    va_start(args, format);
    WriteV(logLevel, format, args);
    va_end(args);
}

void Logger::Info(const char *pFormat, ...) {
    if (baseLogger_ && baseLogger_->LogLevel(ApplicationLog) > LogInfo)
        return;

    va_list args;
    va_start(args, pFormat);
    WriteV(LogInfo, pFormat, args);
    va_end(args);
}

void Logger::Warning(const char *pFormat, ...) {
    if (baseLogger_ && baseLogger_->LogLevel(ApplicationLog) > LogWarning)
        return;

    va_list args;
    va_start(args, pFormat);
    WriteV(LogWarning, pFormat, args);
    va_end(args);
}

void Logger::WriteV(PlatformLogLevel logLevel, const char *pFormat, va_list args) {
    char message[8192];
    vsnprintf(message, 8192, pFormat, args);

    if (baseLogger_) {
        baseLogger_->Log(ApplicationLog, logLevel, message);
    } else {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);

        fprintf(stderr, "[%04d/%02d/%02d %02d:%02d:%02d] %s\n",
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, message);
    }
}
