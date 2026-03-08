#include "InputStream.h"

#include <cassert>
#include <string>

#include "Utility/Exception.h"
#include "Utility/Streams/ChunkBuffer.h"

InputStream::~InputStream() {
    // No need to do any cleanup here, thus we don't have a separate closeInternal() method.
}

size_t InputStream::readAll(std::string *dst) {
    assert(isOpen());
    assert(dst);
    dst->clear();

    if (_size != static_cast<size_t>(-1)) {
        // Sized stream: pre-allocate and read in one go.
        size_t bytesTotal = _size - position();
        if (bytesTotal == 0)
            return 0;
        dst->resize_and_overwrite(bytesTotal, [](char *, size_t n) { return n; }); // Technically UB.
        return read(dst->data(), bytesTotal);
    } else {
        // Unsized stream: accumulate in chunks, materialize once.
        ChunkBuffer chunks;
        size_t bytesTotal = 0;
        while (true) {
            StreamBuffer<char> buf = chunks.allocateChunk();
            size_t bytesRead = read(buf.pos(), buf.remaining());
            bytesTotal += bytesRead;
            if (bytesRead < buf.remaining())
                break;
        }
        dst->resize_and_overwrite(bytesTotal, [](char *, size_t n) { return n; }); // Technically UB.
        chunks.materialize(dst->data(), bytesTotal);
        return bytesTotal;
    }
}

void InputStream::open(Buffer buffer, size_t size, std::string_view displayPath) {
    _buffer = buffer;
    _bufferBase = 0;
    _size = size;
    _isOpen = true;
    _displayPath = displayPath;
}

size_t InputStream::_underflow(void *, size_t, Buffer *buffer) {
    assert(buffer->remaining() == 0);
    return 0;
}

void InputStream::_close() {
    assert(isOpen());
    _buffer.reset(nullptr, nullptr, nullptr);
    _bufferBase = 0;
    _size = static_cast<size_t>(-1);
    _isOpen = false;
    _displayPath = {};
}

size_t InputStream::underflow(void *data, size_t size) {
    assert(size > _buffer.remaining());

    size_t pos = position();

    // Drain the buffer first.
    size_t head;
    if (data) {
        head = _buffer.read(data, _buffer.remaining());
        data = static_cast<char *>(data) + head;
    } else {
        head = _buffer.skip(_buffer.remaining());
    }
    size -= head;

    size_t tail = _underflow(data, size, &_buffer);
    _bufferBase = pos + head + tail - _buffer.used();
    return head + tail;
}

size_t InputStream::readUntilSlow(char delimiter, std::string *dst) {
    assert(dst->empty());

    size_t bytesRead = 0;

    // Consume remaining buffer content (delimiter was not found in it by the fast path).
    _buffer.read(dst, _buffer.remaining());
    bytesRead += dst->size();

    // Refill from source and search.
    while (true) {
        _bufferBase += _buffer.used();
        _underflow(nullptr, 0, &_buffer);
        if (_buffer.remaining() == 0)
            break; // No more data.

        if (const char *p = static_cast<const char *>(memchr(_buffer.pos(), delimiter, _buffer.remaining()))) {
            bytesRead += _buffer.read(dst, p - _buffer.pos());
            bytesRead += _buffer.skip(1);
            return bytesRead;
        }

        bytesRead += _buffer.read(dst, _buffer.remaining());
    }

    return bytesRead;
}

void InputStream::throwReadError(size_t requested, size_t actual) const {
    throw Exception("Failed to read the requested number of bytes from stream '{}', requested {}, got {}", _displayPath, requested, actual);
}

void InputStream::throwSkipError(size_t requested, size_t actual) const {
    throw Exception("Failed to skip the requested number of bytes in stream '{}', requested {}, got {}", _displayPath, requested, actual);
}
