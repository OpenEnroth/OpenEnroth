#include "DistLogSink.h"

#include <utility>
#include <memory>

void DistLogSink::write(const LogCategory &category, LogLevel level, std::string_view message) {
    for (auto &&logSink : _logSinks) {
        logSink->write(category, level, message);
    }
}

void DistLogSink::addLogSink(std::unique_ptr<LogSink> logSink) {
    auto logSinkPtr = logSink.get();
    _logSinks.push_back(std::move(logSink));
}
