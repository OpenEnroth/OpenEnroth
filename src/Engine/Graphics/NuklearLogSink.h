#pragma once

#include <Library/Logger/LogSink.h>

#include <memory>

class NuklearLogSink : public LogSink {
 public:
    void write(const LogCategory& category, LogLevel level, std::string_view message) override;
    static std::unique_ptr<LogSink> createNuklearLogSink();

 private:
    bool _isLogging{};
};
