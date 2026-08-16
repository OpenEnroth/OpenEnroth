#include "Logger.h"

#include <cassert>
#include <string>

#include "LogSink.h"
#include "LogSource.h"
#include "FallbackLogSink.h"

static constinit FallbackLogSink fallbackSink;

constinit Logger Logger::fallbackLogger = Logger(detail::detached, LOG_TRACE, &fallbackSink);

constinit Logger *logger = detail::fallbackLogger();

Logger::Logger(LogLevel level, LogSink *sink) {
    assert(sink);

    _defaultCategory._level = level;
    _defaultCategory._adjustedLevel = LogCategory::adjustLevel(level);
    _sink = sink;

    assert(logger == &fallbackLogger);
    logger = this;
}

Logger::~Logger() {
    if (this == &fallbackLogger)
        return;

    assert(logger == this);
    logger = &fallbackLogger;
}

void Logger::logV(const LogCategory &category, LogLevel level, fmt::string_view fmt, fmt::format_args args) {
    std::string message = fmt::vformat(fmt, args);

    auto guard = std::lock_guard(_mutex);
    _sink->write(category, level, message);
}

LogLevel Logger::level() const {
    return *_defaultCategory._level;
}

void Logger::setLevel(LogLevel level) {
    if (*_defaultCategory._level == level)
        return;

    _defaultCategory._level = level;
    _defaultCategory._adjustedLevel = LogCategory::adjustLevel(level);

    for (const LogCategory *category : LogCategory::instances())
        if (category->_source && !category->_level)
            category->_source->setLevel(level);
}

std::optional<LogLevel> Logger::level(const LogCategory &category) const {
    return category._level;
}

void Logger::setLevel(LogCategory &category, std::optional<LogLevel> level) {
    if (category._level == level)
        return;

    category._level = level;
    category._adjustedLevel = level.transform(&LogCategory::adjustLevel);

    if (category._source) {
        LogLevel effectiveLevel = level ? *level : *_defaultCategory._level;
        category._source->setLevel(effectiveLevel);
    }
}

LogSink *Logger::sink() const {
    return _sink;
}

void Logger::setSink(LogSink *sink) {
    assert(sink);
    _sink = sink;
}
