#include "StreamLogSink.h"

#include <cassert>
#include <memory>
#include <utility>

#include <spdlog/pattern_formatter.h> // NOLINT

#include "LogEnumFunctions.h"

StreamLogSink::StreamLogSink(std::unique_ptr<OutputStream> stream) : _ownStream(std::move(stream)), _stream(_ownStream.get()) {
    assert(_stream);
    initFormatter();
}

StreamLogSink::StreamLogSink(OutputStream *stream): _stream(stream) {
    assert(_stream);
    initFormatter();
}

StreamLogSink::~StreamLogSink() = default;

void StreamLogSink::write(const LogCategory &category, LogLevel level, std::string_view message) {
    spdlog::memory_buf_t formatted;
    _formatter->format(spdlog::details::log_msg(category.name(), translateLogLevel(level), message), formatted);
    _stream->write(formatted.data(), formatted.size());
    _stream->flush();
}

void StreamLogSink::initFormatter() {
    _formatter = std::make_unique<spdlog::pattern_formatter>(); // Using default formatter from spdlog.
}
