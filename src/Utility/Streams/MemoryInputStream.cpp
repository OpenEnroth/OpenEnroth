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
    pos_ = static_cast<const char *>(data);
    end_ = pos_ + size;
}

size_t MemoryInputStream::read(void *data, size_t size) {
    assert(pos_);

    size_t result = std::min(size, static_cast<size_t>(end_ - pos_));

    memcpy(data, pos_, result);
    pos_ += result;
    return result;
}

size_t MemoryInputStream::skip(size_t size) {
    assert(pos_);

    size_t result = std::min(size, static_cast<size_t>(end_ - pos_));
    pos_ += result;
    return result;
}

void MemoryInputStream::close() {
    reset(nullptr, 0);
}
