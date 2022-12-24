#pragma once

#include <cstdio>

#include "Platform/PlatformLogger.h"

class Logger {
 public:
    // TODO(captainurist): this should go to ctor, but that's not doable right now because of a shitload of static
    // variables that call EngineIoc::ResolveLogger.
    PlatformLogger *BaseLogger() const;
    void SetBaseLogger(PlatformLogger *baseLogger);

    void Write(PlatformLogLevel logLevel, const char *format, ...);

    void Info(const char *pFormat, ...);
    void Warning(const char *pFormat, ...);

 private:
    void WriteV(PlatformLogLevel logLevel, const char *pFormat, va_list args);

 private:
    PlatformLogger *baseLogger_ = nullptr;
};
