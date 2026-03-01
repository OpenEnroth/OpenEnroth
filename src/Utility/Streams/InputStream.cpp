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

    // Drain the buffer first.
    size_t result = std::min(bufferRemaining(), maxSize);
    dst->append(_pos, result);
    _pos += result;

    // Let the subclass read the rest.
    return result + _readAll(dst, maxSize - result);
}

InputStream::InputStream(const void *bufferStart, const void *bufferEnd, std::string_view displayPath) {
    open(bufferStart, bufferEnd, displayPath);
}

void InputStream::open(const void *bufferStart, const void *bufferEnd, std::string_view displayPath) {
    assert(bufferEnd >= bufferStart);

    _pos = static_cast<const char *>(bufferStart);
    _end = static_cast<const char *>(bufferEnd);
    _isOpen = true;
    _displayPath = displayPath;
}

void InputStream::open(std::string_view displayPath) {
    _pos = nullptr;
    _end = nullptr;
    _isOpen = true;
    _displayPath = displayPath;
}

size_t InputStream::_underflow(void *, size_t, const void **bufferStart, const void **bufferEnd) {
    assert(bufferRemaining() == 0);
    *bufferStart = nullptr;
    *bufferEnd = nullptr;
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
    _pos = nullptr;
    _end = nullptr;
    _isOpen = false;
    _displayPath = {};
}

size_t InputStream::readSlow(void *data, size_t size) {
    assert(data);
    assert(size > bufferRemaining());

    size_t result = bufferRemaining();
    memcpy(data, _pos, result);
    _pos = _end;

    result += _underflow(static_cast<char *>(data) + result, size - result,
                         reinterpret_cast<const void **>(&_pos), reinterpret_cast<const void **>(&_end));
    return result;
}

size_t InputStream::skipSlow(size_t size) {
    assert(size > bufferRemaining());

    size_t result = bufferRemaining();
    _pos = _end;

    result += _underflow(nullptr, size - result,
                         reinterpret_cast<const void **>(&_pos), reinterpret_cast<const void **>(&_end));
    return result;
}

size_t InputStream::readUntilSlow(char delimiter, std::string *dst, size_t maxSize) {
    assert(dst->empty());

    size_t result = 0;

    // Consume remaining buffer content (delimiter was not found in it by the fast path).
    size_t bytes = std::min(bufferRemaining(), maxSize);
    dst->append(_pos, bytes);
    _pos += bytes;
    result += bytes;

    // Refill from source and search.
    while (result < maxSize) {
        _underflow(nullptr, 0, reinterpret_cast<const void **>(&_pos), reinterpret_cast<const void **>(&_end));
        if (_pos == _end)
            break; // No more data.

        size_t available = std::min(bufferRemaining(), maxSize - result);
        if (const char *pos = static_cast<const char *>(memchr(_pos, delimiter, available))) {
            size_t size = pos - _pos;
            dst->append(_pos, size);
            _pos = pos + 1;
            return result + size + 1;
        }

        dst->append(_pos, available);
        _pos += available;
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
