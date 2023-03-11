#include "MemoryInputStream.h"

#include <cassert>
#include <cstring>
#include <algorithm>

MemoryInputStream::MemoryInputStream() {
    Reset(nullptr, 0);
}

MemoryInputStream::MemoryInputStream(const void *data, size_t size) {
    Reset(data, size);
}

MemoryInputStream::~MemoryInputStream() {}

void MemoryInputStream::Reset(const void *data, size_t size) {
    pos_ = static_cast<const char *>(data);
    end_ = pos_ + size;
}

size_t MemoryInputStream::Read(void *data, size_t size) {
    assert(pos_);

    size_t result = std::min(size, static_cast<size_t>(end_ - pos_));

    memcpy(data, pos_, result);
    pos_ += result;
    return result;
}

size_t MemoryInputStream::Skip(size_t size) {
    assert(pos_);

    size_t result = std::min(size, static_cast<size_t>(end_ - pos_));
    pos_ += result;
    return result;
}

void MemoryInputStream::Close() {
    Reset(nullptr, 0);
}
