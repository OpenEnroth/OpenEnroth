#pragma once

#include <Platform/PlatformLogger.h>

class SdlLogger: public PlatformLogger {
 public:
    virtual void SetLogLevel(PlatformLogCategory category, PlatformLogLevel logLevel) override;
    virtual PlatformLogLevel LogLevel(PlatformLogCategory category) const override;

    virtual void Log(PlatformLogLevel logLevel, const char* message) override;

    void LogSdlError(const char *sdlFunctionName);
};
