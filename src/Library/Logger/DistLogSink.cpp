#include "DistLogSink.h"

#include <utility>
#include <memory>

void DistLogSink::write(const LogCategory &category, LogLevel level, std::string_view message) {
    for (auto &&logSink : _logSinks) {
        logSink->write(category, level, message);
    }
}

std::function<void()> DistLogSink::addLogSink(std::unique_ptr<LogSink> logSink) {
    auto logSinkPtr = logSink.get();
    _logSinks.push_back(std::move(logSink));

    std::function<void()> cleanup = [logSinkPtr, this]() {
        std::erase_if(_logSinks, [logSinkPtr](auto &&internalLogSink) {
            return internalLogSink.get() == logSinkPtr;
        });
    };

    return cleanup;
}
