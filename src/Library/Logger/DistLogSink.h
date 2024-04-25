#pragma once

#include <memory>
#include <vector>
#include <functional>

#include "LogSink.h"

class DistLogSink : public LogSink {
 public:
    void write(const LogCategory &category, LogLevel level, std::string_view message) override;

    std::function<void()> addLogSink(std::unique_ptr<LogSink> logSink);

 private:
     std::vector<std::unique_ptr<LogSink>> _logSinks;
};
