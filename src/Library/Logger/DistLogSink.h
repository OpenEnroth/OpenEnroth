#pragma once

#include <memory>
#include <vector>

#include "LogSink.h"

class DistLogSink : public LogSink {
 public:
    void write(const LogCategory &category, LogLevel level, std::string_view message) override;

    void addLogSink(std::unique_ptr<LogSink> logSink);

 private:
     std::vector<std::unique_ptr<LogSink>> _logSinks;
};
