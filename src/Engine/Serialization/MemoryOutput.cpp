#include "MemoryOutput.h"

#include <cassert>
#include <cstring>
#include <algorithm>

static const size_t InitialSpanSize = 1024;
static const size_t MaxSpanSizeShift = 10; // So max size of a single span is 1Mb

void MemoryOutput::Reset() {
    for (auto &span : spans_)
        delete[] span.data();
    spans_.clear();
    pos_ = 0;
}

Blob MemoryOutput::Finish() {
    if (spans_.size() == 0)
        return Blob();

    spans_.back() = spans_.back().subspan(0, pos_);

    size_t size = 0;
    for (const auto &span : spans_)
        size += span.size();

    Blob result = Blob::Allocate(size);

    size_t pos = 0;
    for (const auto &span : spans_) {
        memcpy(static_cast<char *>(result.data()) + pos, span.data(), span.size());
        pos += span.size();
    }

    Reset();

    return result;
}

void *MemoryOutput::Allocate(size_t size) {
    auto allocateNextSpan = [this] (size_t minSize) {
        size_t size = std::max(minSize, InitialSpanSize << std::min(MaxSpanSizeShift, spans_.size()));
        spans_.emplace_back(new std::byte[size], size);
    };

    if (spans_.empty())
        allocateNextSpan(size);

    if (spans_.back().size() < pos_ + size) {
        spans_.back() = spans_.back().subspan(0, pos_);
        allocateNextSpan(size);
        pos_ = 0;
    }

    assert(spans_.back().size() >= pos_ + size);

    void *result = spans_.back().data() + pos_;
    pos_ += size;
    return result;
}
