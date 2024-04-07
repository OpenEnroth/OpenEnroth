#include "DistLogSink.h"

#include <utility>

void DistLogSink::write(const LogCategory &category, LogLevel level, std::string_view message) {
    for (auto &&logSink : _logSinks) {
        logSink->write(category, level, message);
    }
}

void DistLogSink::addLogSink(std::unique_ptr<LogSink> logSink) {
    _logSinks.push_back(std::move(logSink));
}
