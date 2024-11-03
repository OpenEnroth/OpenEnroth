#include "StreamLogSink.h"

#include <cassert>
#include <memory>

#include <spdlog/pattern_formatter.h> // NOLINT

#include "LogEnumFunctions.h"

StreamLogSink::StreamLogSink(OutputStream *stream): _stream(stream) {
    assert(stream);
    _formatter = std::make_unique<spdlog::pattern_formatter>(); // Using default formatter from spdlog.
}

StreamLogSink::~StreamLogSink() = default;

void StreamLogSink::write(const LogCategory &category, LogLevel level, std::string_view message) {
    spdlog::memory_buf_t formatted;
    _formatter->format(spdlog::details::log_msg(category.name(), translateLogLevel(level), message), formatted);
    _stream->write(formatted.data(), formatted.size());
    _stream->flush();
}
