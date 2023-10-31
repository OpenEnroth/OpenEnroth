#pragma once

#include <string>
#include <vector>

#include "LogSink.h"
#include "Logger.h"

class BufferLogSink : public LogSink {
 public:
    virtual void write(const LogCategory &category, LogLevel level, std::string_view message) override {
        _buffer.push_back({&category, level, std::string(message)});
    }

    void flush(Logger *target) {
        for (const LogMessage &message : _buffer)
            target->log(*message.category, message.level, "{}", message.message);
        _buffer.clear();
    }

 private:
    struct LogMessage {
        const LogCategory *category = nullptr;
        LogLevel level = LOG_TRACE;
        std::string message;
    };

 private:
    std::vector<LogMessage> _buffer;
};
