#pragma once

#include <memory>

#include "Library/Logger/LogEnums.h"

class OutputStream;
class FileSystem;
class LogSink;
class DistLogSink;
class BufferLogSink;
class StreamLogSink;
class Logger;

class LogStarter {
 public:
    LogStarter();
    ~LogStarter();

    void initPrimary(); // Can use global logger after this.
    void initSecondary(FileSystem *userFs); // Start writing filesystem log.
    void initFinal(LogLevel logLevel); // Set log level & finalize logger init.

    DistLogSink *rootSink() const;

 private:
    enum class Stage {
        STAGE_INITIAL,
        STAGE_PRIMARY,
        STAGE_SECONDARY,
        STAGE_FINAL
    };
    using enum Stage;

 private:
    Stage _stage = STAGE_INITIAL;
    std::unique_ptr<BufferLogSink> _bufferLogSink;
    std::unique_ptr<LogSink> _defaultLogSink;
    std::unique_ptr<OutputStream> _userLogStream;
    std::unique_ptr<StreamLogSink> _userLogSink;
    std::unique_ptr<DistLogSink> _rootLogSink;
    std::unique_ptr<Logger> _logger;
};
