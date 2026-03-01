#include "BlobOutputStream.h"

#include <cassert>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>

#include "Utility/Memory/FreeDeleter.h"

BlobOutputStream::BlobOutputStream(Blob *target, std::string_view displayPath) {
    open(target, displayPath);
}

BlobOutputStream::~BlobOutputStream() {
    closeInternal();
}

void BlobOutputStream::open(Blob *target, std::string_view displayPath) {
    assert(target);
    _target = target;
    _chunks.clear();
    OutputStream::open(displayPath);
}

void BlobOutputStream::_overflow(const void *data, size_t size, void **bufferStart, void **bufferEnd) {
    assert(bufferRemaining() == 0);

    // Geometric growth: 1KB -> 2KB -> 4KB -> ... -> 1MB (cap), but at least enough for the overflow data.
    size_t chunkSize = std::max(size, _chunks.empty() ? 1024 : std::min<size_t>(_chunks.back().size * 2, 1024 * 1024));

    // Allocate a single chunk: overflow data at the start, remaining space is the new buffer.
    std::unique_ptr<char, FreeDeleter> buf(static_cast<char *>(malloc(chunkSize)));
    memcpy(buf.get(), data, size);
    _chunks.push_back({std::move(buf), chunkSize});
    *bufferStart = _chunks.back().data.get() + size;
    *bufferEnd = _chunks.back().data.get() + chunkSize;
}

void BlobOutputStream::_flush() {
    assert(isOpen());
    *_target = materialize();
}

void BlobOutputStream::_close() {
    assert(isOpen());
    closeInternal();
    OutputStream::_close();
}

void BlobOutputStream::closeInternal() {
    if (!isOpen())
        return;

    if (_chunks.empty()) {
        *_target = Blob().withDisplayPath(displayPath());
    } else if (_chunks.size() == 1) {
        // Single chunk: move the malloc'd memory directly into the Blob, no copy needed.
        *_target = Blob::fromMalloc(std::move(_chunks.front().data), _chunks.front().size - bufferRemaining())
            .withDisplayPath(displayPath());
    } else {
        *_target = materialize();
    }

    _target = nullptr;
    _chunks.clear();
}

Blob BlobOutputStream::materialize() {
    // Compute total used size. All chunks are fully used except the last one (the buffer chunk).
    size_t total = 0;
    for (const Chunk &chunk : _chunks)
        total += chunk.size;
    total -= bufferRemaining();
    if (total == 0)
        return Blob().withDisplayPath(displayPath());

    // Concatenate all chunks into a single malloc'd buffer.
    std::unique_ptr<char, FreeDeleter> result(static_cast<char *>(malloc(total)));
    size_t remaining = total;
    size_t offset = 0;
    for (const Chunk &chunk : _chunks) {
        size_t bytes = std::min(chunk.size, remaining);
        memcpy(result.get() + offset, chunk.data.get(), bytes);
        offset += bytes;
        remaining -= bytes;
    }

    return Blob::fromMalloc(std::move(result), total).withDisplayPath(displayPath());
}
