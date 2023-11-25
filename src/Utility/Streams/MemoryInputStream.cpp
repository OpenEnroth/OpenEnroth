#include "MemoryInputStream.h"

#include <cassert>
#include <cstring>
#include <algorithm>

MemoryInputStream::MemoryInputStream() {
    reset(nullptr, 0);
}

MemoryInputStream::MemoryInputStream(const void *data, size_t size) {
    reset(data, size);
}

MemoryInputStream::~MemoryInputStream() {}

void MemoryInputStream::reset(const void *data, size_t size) {
    _begin = static_cast<const char *>(data);
    _pos = _begin;
    _end = _pos + size;
}

size_t MemoryInputStream::read(void *data, size_t size) {
    assert(_pos);

    size_t result = std::min(size, static_cast<size_t>(_end - _pos));

    memcpy(data, _pos, result);
    _pos += result;
    return result;
}

size_t MemoryInputStream::skip(size_t size) {
    assert(_pos);

    size_t result = std::min(size, static_cast<size_t>(_end - _pos));
    _pos += result;
    return result;
}

void MemoryInputStream::close() {
    reset(nullptr, 0);
}

void MemoryInputStream::seek(size_t pos) {
    _pos = _begin + std::min(pos, static_cast<size_t>(_end - _begin));
}

size_t MemoryInputStream::position() const {
    return _pos - _begin;
}
