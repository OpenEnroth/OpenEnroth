#include "MemoryInputStream.h"

#include <cassert>
#include <cstring>
#include <algorithm>
#include <string>

MemoryInputStream::MemoryInputStream() {
    reset(nullptr, 0);
}

MemoryInputStream::MemoryInputStream(const void *data, size_t size, std::string_view displayPath) {
    reset(data, size, displayPath);
}

MemoryInputStream::~MemoryInputStream() {}

void MemoryInputStream::reset(const void *data, size_t size, std::string_view displayPath) {
    _begin = static_cast<const char *>(data);
    _pos = _begin;
    _end = _pos + size;
    _displayPath = displayPath;
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

std::string MemoryInputStream::displayPath() const {
    return _displayPath;
}

void MemoryInputStream::seek(ssize_t pos) {
    assert(_pos);

    _pos = _begin + std::min(pos, size());
}

ssize_t MemoryInputStream::position() const {
    assert(_pos);

    return _pos - _begin;
}

ssize_t MemoryInputStream::size() const {
    assert(_pos);

    return _end - _begin;
}
