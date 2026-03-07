#include "OutputStream.h"

#include <cassert>

OutputStream::~OutputStream() {
    // No need to do any cleanup here, thus we don't have a separate closeInternal() method.
}

void OutputStream::open(Buffer buffer, std::string_view displayPath) {
    _buffer = buffer;
    _bufferBase = 0;
    _isOpen = true;
    _displayPath = displayPath;
}

void OutputStream::_close(Buffer *buffer) {
    assert(isOpen());
    _buffer.reset(nullptr, nullptr, nullptr);
    _bufferBase = 0;
    _isOpen = false;
    _displayPath = {};
}

void OutputStream::overflow(const void *data, size_t size) {
    assert(size > _buffer.remaining());
    size_t pos = position();
    _overflow(data, size, &_buffer);
    _bufferBase = pos + size - _buffer.used();
}
