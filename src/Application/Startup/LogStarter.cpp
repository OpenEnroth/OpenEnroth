#include "LogStarter.h"

#include <cassert>
#include <string>
#include <memory>

#include "Library/FileSystem/Interface/FileSystem.h"
#include "Library/Logger/RotatingLogSink.h"
#include "Library/Logger/DistLogSink.h"
#include "Library/Logger/BufferLogSink.h"

LogStarter::LogStarter() {
    _rootLogSink = std::make_unique<DistLogSink>();
    _logger = std::make_unique<Logger>(LOG_TRACE, _rootLogSink.get());

    // We write to a buffer until the desired log level is known.
    _bufferLogSink = std::make_unique<BufferLogSink>();
    _rootLogSink->addLogSink(_bufferLogSink.get());
}

LogStarter::~LogStarter() {
    // We never got around to finding out the desired log level => flush everything at LOG_TRACE.
    if (!_initialized) {
        try {
            initialize(nullptr, LOG_TRACE);
        } catch (...) {
            // Nothing we can do here.
        }
    }
}

void LogStarter::initialize(FileSystem *userFs, LogLevel logLevel) {
    assert(!_initialized);
    _initialized = true;

    // Create & install default log sink.
    _defaultLogSink = LogSink::createDefaultSink();
    _rootLogSink->addLogSink(_defaultLogSink.get());

    // Set up filesystem logging. We skip this on LOG_NONE b/c we don't want any FS changes in this case.
    if (userFs && logLevel != LOG_NONE) {
        try {
            _userLogSink = std::make_unique<RotatingLogSink>("logs/openenroth.log", userFs);
            _rootLogSink->addLogSink(_userLogSink.get());
        } catch (const std::exception &e) {
            _logger->log(LOG_ERROR, "Could not open log file for writing: {}", e.what());
            _userLogSink.reset();
        }
    }

    // Then init log level & flush.
    _rootLogSink->removeLogSink(_bufferLogSink.get());
    _logger->setLevel(logLevel);
    _bufferLogSink->flush(_logger.get());
    _bufferLogSink.reset();
}

DistLogSink *LogStarter::rootSink() const {
    return _rootLogSink.get();
}
