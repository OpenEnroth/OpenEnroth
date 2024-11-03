#pragma once

#include <memory>

#include "Library/Logger/LogEnums.h"

class OutputStream;
class FileSystem;
class LogSink;
class DistLogSink;
class BufferLogSink;
class RotatingLogSink;
class Logger;

class LogStarter {
 public:
    LogStarter();
    ~LogStarter();

    void initialize(FileSystem *userFs, LogLevel logLevel); // Set log level & finalize logger init.

    DistLogSink *rootSink() const;

 private:
    bool _initialized = false;
    std::unique_ptr<LogSink> _nullSink;
    std::unique_ptr<BufferLogSink> _bufferLogSink;
    std::unique_ptr<LogSink> _defaultLogSink;
    std::unique_ptr<RotatingLogSink> _userLogSink;
    std::unique_ptr<DistLogSink> _rootLogSink;
    std::unique_ptr<Logger> _logger;
};
