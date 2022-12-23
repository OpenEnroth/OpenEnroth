#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <cassert>

#include "Log.h"

PlatformLogger *Log::BaseLogger() const {
    return baseLogger_;
}

void Log::SetBaseLogger(PlatformLogger *baseLogger) {
    baseLogger_ = baseLogger;
}

void Log::Info(const char *pFormat, ...) {
    if (baseLogger_ && baseLogger_->LogLevel(ApplicationLog) > LogInfo)
        return;

    va_list args;
    va_start(args, pFormat);
    WriteV(LogInfo, pFormat, args);
    va_end(args);
}

void Log::Warning(const char *pFormat, ...) {
    if (baseLogger_ && baseLogger_->LogLevel(ApplicationLog) > LogWarning)
        return;

    va_list args;
    va_start(args, pFormat);
    WriteV(LogWarning, pFormat, args);
    va_end(args);
}

void Log::WriteV(PlatformLogLevel logLevel, const char *pFormat, va_list args) {
    char message[8192];
    vsnprintf(message, 8192, pFormat, args);

    if (baseLogger_) {
        baseLogger_->Log(logLevel, message);
    } else {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);

        fprintf(stderr, "[%04d/%02d/%02d %02d:%02d:%02d] %s\n",
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, message);
    }
}
