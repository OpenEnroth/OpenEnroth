#pragma once

#include <Platform/PlatformLogger.h>

class SdlLogger: public PlatformLogger {
 public:
    virtual void SetLogLevel(PlatformLogCategory category, PlatformLogLevel logLevel) override;
    virtual PlatformLogLevel LogLevel(PlatformLogCategory category) const override;

    virtual void Log(PlatformLogCategory category, PlatformLogLevel logLevel, const char *message) override;
};
