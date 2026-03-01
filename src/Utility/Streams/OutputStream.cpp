#include "OutputStream.h"

#include <cassert>

OutputStream::~OutputStream() = default;

void OutputStream::_close() {
    assert(isOpen());
    closeInternal();
}

void OutputStream::open(std::string_view displayPath) {
    _displayPath = displayPath;
    _pos = nullptr;
    _end = nullptr;
    _isOpen = true;
}

void OutputStream::open(void *bufferStart, void *bufferEnd, std::string_view displayPath) {
    assert(bufferEnd >= bufferStart);
    _displayPath = displayPath;
    _pos = static_cast<char *>(bufferStart);
    _end = static_cast<char *>(bufferEnd);
    _isOpen = true;
}

void OutputStream::closeInternal() {
    _pos = nullptr;
    _end = nullptr;
    _isOpen = false;
    _displayPath = {};
}

void OutputStream::writeSlow(const void *data, size_t size) {
    auto src = static_cast<const char *>(data);

    // Fill remaining buffer.
    size_t available = bufferRemaining();
    memcpy(_pos, src, available);
    _pos = _end;
    src += available;
    size -= available;

    _overflow(src, size, reinterpret_cast<void **>(&_pos), reinterpret_cast<void **>(&_end));
}
