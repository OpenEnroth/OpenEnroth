#pragma once

#include <memory>

#include "Utility/Streams/OutputStream.h"

#include "LogSink.h"

namespace spdlog { class formatter; }

class StreamLogSink : public LogSink {
 public:
    explicit StreamLogSink(std::unique_ptr<OutputStream> stream);
    explicit StreamLogSink(OutputStream *stream);
    virtual ~StreamLogSink();

    virtual void write(const LogCategory &category, LogLevel level, std::string_view message) override;

 private:
    void initFormatter();

 private:
    std::unique_ptr<OutputStream> _ownStream;
    OutputStream *_stream = nullptr;
    std::unique_ptr<spdlog::formatter> _formatter;
};
