#include "StringOutputStream.h"

#include <cassert>
#include <algorithm>
#include <cstring>
#include <string>

StringOutputStream::StringOutputStream(std::string *target, std::string_view displayPath) {
    open(target, displayPath);
}

StringOutputStream::~StringOutputStream() {
    closeInternal();
}

void StringOutputStream::open(std::string *target, std::string_view displayPath) {
    assert(target);
    _target = target;
    _target->clear();
    OutputStream::open(displayPath);
}

void StringOutputStream::_overflow(const void *data, size_t size, void **bufferStart, void **bufferEnd) {
    assert(bufferRemaining() == 0);

    // Geometric growth: 1KB -> 2KB -> ... -> 1MB (cap), but at least enough for the overflow data.
    size_t oldSize = _target->size();
    size_t chunkSize = std::max(size, oldSize == 0 ? 1024 : std::min<size_t>(oldSize, 1024 * 1024));

    // Grow the string: overflow data at the start, remaining space is the new buffer.
    _target->resize_and_overwrite(oldSize + chunkSize,
        [data, size, offset = oldSize](char *buf, size_t n) {
            memcpy(buf + offset, data, size);
            return n;
        });
    *bufferStart = _target->data() + oldSize + size;
    *bufferEnd = _target->data() + oldSize + chunkSize;
}

void StringOutputStream::_flush() {
    assert(isOpen());
    _target->resize(_target->size() - bufferRemaining());
}

void StringOutputStream::_close() {
    assert(isOpen());
    closeInternal();
    OutputStream::_close();
}

void StringOutputStream::closeInternal() {
    if (!isOpen())
        return;

    _target->resize(_target->size() - bufferRemaining());
    _target = nullptr;
}
