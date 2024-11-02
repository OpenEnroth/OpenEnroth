#include "LogStarter.h"

#include <cassert>
#include <string>
#include <memory>

#include "Library/FileSystem/Interface/FileSystem.h"
#include "Library/Logger/RotatingLogSink.h"
#include "Library/Logger/DistLogSink.h"
#include "Library/Logger/BufferLogSink.h"

LogStarter::LogStarter() = default;

LogStarter::~LogStarter() {
    // We never got around to finding out the desired log level => flush everything at LOG_TRACE.
    if (_stage != STAGE_FINAL && _stage != STAGE_INITIAL) {
        _stage = STAGE_SECONDARY;
        try {
            initFinal(LOG_TRACE);
        } catch (...) {
            // Nothing we can do here.
        }
    }
}

void LogStarter::initPrimary() {
    assert(_stage == STAGE_INITIAL);
    _stage = STAGE_PRIMARY;

    _rootLogSink = std::make_unique<DistLogSink>();
    _logger = std::make_unique<Logger>(LOG_TRACE, _rootLogSink.get());

    // We write to a buffer until the desired log level is known.
    _bufferLogSink = std::make_unique<BufferLogSink>();
    _rootLogSink->addLogSink(_bufferLogSink.get());
}

void LogStarter::initSecondary(FileSystem *userFs) {
    assert(_stage == STAGE_PRIMARY);
    _stage = STAGE_SECONDARY;

    try {
        _userLogSink = std::make_unique<RotatingLogSink>("logs/openenroth.log", userFs);
    } catch (const std::exception &e) {
        _logger->log(LOG_ERROR, "Could not open log file for writing: {}", e.what());
        _userLogSink.reset();
    }
}

void LogStarter::initFinal(LogLevel logLevel) {
    assert(_stage == STAGE_SECONDARY);
    _stage = STAGE_FINAL;

    // Create proper log sinks.
    _defaultLogSink = LogSink::createDefaultSink();
    _rootLogSink->addLogSink(_defaultLogSink.get());
    if (_userLogSink)
        _rootLogSink->addLogSink(_userLogSink.get());

    // Then init log level & flush.
    _rootLogSink->removeLogSink(_bufferLogSink.get());
    _logger->setLevel(logLevel);
    _bufferLogSink->flush(_logger.get());
    _bufferLogSink.reset();
}

DistLogSink *LogStarter::rootSink() const {
    return _rootLogSink.get();
}
