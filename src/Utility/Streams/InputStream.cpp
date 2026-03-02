#include "InputStream.h"

#include <cassert>
#include <algorithm>
#include <string>

#include "Utility/Exception.h"

InputStream::~InputStream() = default;

size_t InputStream::readAll(std::string *dst, size_t maxSize) {
    assert(isOpen());
    assert(dst);
    dst->clear();

    size_t result = std::min(_buffer.remaining(), maxSize);
    _buffer.read(dst, result); // Drain the buffer first.
    return result + _readAll(dst, maxSize - result); // Let the subclass read the rest.
}

InputStream::InputStream(Buffer buffer, std::string_view displayPath) {
    open(buffer, displayPath);
}

void InputStream::open(Buffer buffer, std::string_view displayPath) {
    _buffer = buffer;
    _isOpen = true;
    _displayPath = displayPath;
}

size_t InputStream::_underflow(void *, size_t, Buffer *buffer) {
    assert(buffer->remaining() == 0);
    buffer->reset(nullptr, nullptr, nullptr);
    return 0;
}

size_t InputStream::_readAll(std::string *, size_t) {
    return 0;
}

void InputStream::_close() {
    assert(isOpen());
    closeInternal();
}

void InputStream::closeInternal() {
    _buffer.reset(nullptr, nullptr, nullptr);
    _isOpen = false;
    _displayPath = {};
}

size_t InputStream::readSlow(void *data, size_t size) {
    assert(data);
    assert(size > _buffer.remaining());

    size_t result = _buffer.read(data, _buffer.remaining());

    result += _underflow(static_cast<char *>(data) + result, size - result, &_buffer);
    return result;
}

size_t InputStream::skipSlow(size_t size) {
    assert(size > _buffer.remaining());

    size_t result = _buffer.skip(_buffer.remaining());

    result += _underflow(nullptr, size - result, &_buffer);
    return result;
}

size_t InputStream::readUntilSlow(char delimiter, std::string *dst, size_t maxSize) {
    assert(dst->empty());

    size_t result = 0;

    // Consume remaining buffer content (delimiter was not found in it by the fast path).
    size_t bytes = std::min(_buffer.remaining(), maxSize);
    _buffer.read(dst, bytes);
    result += bytes;

    // Refill from source and search.
    while (result < maxSize) {
        _underflow(nullptr, 0, &_buffer);
        if (_buffer.remaining() == 0)
            break; // No more data.

        size_t available = std::min(_buffer.remaining(), maxSize - result);
        if (const char *p = static_cast<const char *>(memchr(_buffer.pos(), delimiter, available))) {
            size_t size = p - _buffer.pos();
            _buffer.read(dst, size);
            _buffer.skip(1);
            return result + size + 1;
        }

        _buffer.read(dst, available);
        result += available;
    }

    return result;
}

void InputStream::throwReadError(size_t requested, size_t actual) {
    throw Exception("Failed to read the requested number of bytes from a stream, requested {}, got {}", requested, actual);
}

void InputStream::throwSkipError(size_t requested, size_t actual) {
    throw Exception("Failed to skip the requested number of bytes in a stream, requested {}, got {}", requested, actual);
}
