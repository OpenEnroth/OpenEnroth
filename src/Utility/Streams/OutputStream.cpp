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

    // Rebased on every path - `accepted` stays zero if the write throws, and part of the buffer may have gone out
    // by then, which must not move `position()` backwards.
    size_t pos = position();
    size_t accepted = 0;
    MM_AT_SCOPE_EXIT(_bufferBase = pos + accepted - _buffer.used());
    _overflow(&_buffer, data, size);
    accepted = size;
}
