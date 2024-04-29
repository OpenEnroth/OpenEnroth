#pragma once

#include <vector>

#include "LogSink.h"

/**
 * Log sink that distributes what's written into it into other log sinks.
 */
class DistLogSink : public LogSink {
 public:
    void write(const LogCategory &category, LogLevel level, std::string_view message) override;

    void addLogSink(LogSink *logSink);
    void removeLogSink(LogSink *logSink);

 private:
     std::vector<LogSink *> _logSinks;
};
