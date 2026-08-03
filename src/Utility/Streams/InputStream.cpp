#include "InputStream.h"

#include <cassert>
#include <span>
#include <string>

#include "Utility/Exception.h"
#include "Utility/Memory/MemoryScratchpad.h"

InputStream::~InputStream() = default;

size_t InputStream::readAll(std::string *dst) {
    assert(isOpen());
    assert(dst);
    dst->clear();

    if (_size != static_cast<size_t>(-1)) {
        // Sized stream: pre-allocate from the size and read it in one go. Note that `position()` can legitimately be
        // past `_size`, e.g. after a large read from a file that has grown, so this must not underflow.
        if (_size > position()) {
            size_t bytesHint = _size - position();
            dst->resize_and_overwrite(bytesHint, [](char *, size_t n) { return n; }); // Technically UB, but throwing
            dst->resize(read(dst->data(), bytesHint));                                // from the callback is also UB.
        }

        // The size is only a hint - it's sampled when the stream is opened and the file can be appended to afterwards,
        // and `st_size` is plain wrong for some files (procfs reports 0 for files that do have contents). So keep
        // reading until the stream actually ends. For a stream that ended where the size said, this is a single
        // empty read.
        char chunk[4096];
        while (size_t bytesRead = read(chunk, sizeof(chunk)))
            dst->append(chunk, bytesRead);

        return dst->size();
    } else {
        // Unsized stream: accumulate in chunks, materialize once.
        MemoryScratchpad scratchpad;
        size_t bytesTotal = 0;
        while (true) {
            std::span<char> chunk = scratchpad.next();
            size_t bytesRead = read(chunk.data(), chunk.size());
            bytesTotal += bytesRead;
            if (bytesRead < chunk.size())
                break;
        }
        dst->resize_and_overwrite(bytesTotal, [&scratchpad](char *buffer, size_t size) {
            scratchpad.materialize(buffer, size);
            return size;
        });
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

    // Note that the buffer has to be reset even though there's no more data. `readUntilSlow` folds `buffer->used()`
    // into `_bufferBase` before calling us, so leaving a non-zero `used()` here would double-count it into
    // `position()` - and `position()` running past `size()` then makes the `size() - position()` checks in the
    // binary deserializer underflow.
    buffer->commit();
    return 0;
}

void InputStream::_close(bool /*canThrow*/) {
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
