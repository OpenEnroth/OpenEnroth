#pragma once

#include "Library/Logger/LogSource.h"

class FFmpegLogSource : public LogSource {
 public:
    virtual LogLevel level() const override;
    virtual void setLevel(LogLevel level) override;

    static LogLevel translateFFmpegLogLevel(int level);
    static int translateLoggerLogLevel(LogLevel level);
};
