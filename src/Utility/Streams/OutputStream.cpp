#include "OutputStream.h"

#include <cassert>

OutputStream::~OutputStream() = default;

void OutputStream::open(Buffer buffer, std::string_view displayPath) {
    _buffer = buffer;
    _bufferBase = 0;
    _isOpen = true;
    _displayPath = displayPath;
}

void OutputStream::_close(Buffer *buffer, bool /*canThrow*/) {
    assert(isOpen());
    _buffer.reset(nullptr, nullptr, nullptr);
    _bufferBase = 0;
    _isOpen = false;
    _displayPath = {};
}


void OutputStream::overflow(const void *data, size_t size) {
    assert(size > _buffer.remaining());

    // Rebased on every path - a write that throws can still have pushed part of the data out, and `position()` has to
    // move forward by exactly what the implementation reports it took.
    size_t pos = position();
    size_t bytesAccepted = 0;
    MM_AT_SCOPE_EXIT(_bufferBase = pos + bytesAccepted - _buffer.used());
    _overflow(&_buffer, data, size, &bytesAccepted);
}
