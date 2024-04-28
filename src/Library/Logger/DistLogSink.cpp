#include "DistLogSink.h"

void DistLogSink::write(const LogCategory &category, LogLevel level, std::string_view message) {
    for (auto &&logSink : _logSinks)
        logSink->write(category, level, message);
}

void DistLogSink::addLogSink(LogSink *logSink) {
    _logSinks.push_back(logSink);
}

void DistLogSink::removeLogSink(LogSink *logSink) {
    std::erase(_logSinks, logSink);
}
