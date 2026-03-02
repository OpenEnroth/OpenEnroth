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
    base_type::open({}, displayPath);
}

void BlobOutputStream::_overflow(const void *data, size_t size, Buffer *buffer) {
    assert(size > buffer->remaining());

    const char *src = static_cast<const char *>(data);

    // Fill the remaining space in the current chunk.
    size_t head = buffer->write(src, buffer->remaining());
    src += head;
    size -= head;

    // Geometric growth: 1KB -> 2KB -> 4KB -> ... -> 1MB (cap), but at least enough for the remaining overflow data.
    size_t chunkSize = std::max(size, _chunks.empty() ? 1024 : std::min<size_t>(_chunks.back().size * 2, 1024 * 1024));

    // Allocate a new chunk: remaining overflow data at the start, rest is the new buffer.
    std::unique_ptr<char, FreeDeleter> buf(static_cast<char *>(malloc(chunkSize)));
    memcpy(buf.get(), src, size);
    _chunks.push_back({std::move(buf), chunkSize});
    char *chunkStart = _chunks.back().data.get();
    buffer->reset(chunkStart, chunkStart + size, chunkStart + chunkSize);
}

void BlobOutputStream::_flush(Buffer *buffer) {
    *_target = materialize();
    buffer->chop();
}

void BlobOutputStream::_close() {
    assert(isOpen());
    closeInternal();
    base_type::_close();
}

void BlobOutputStream::closeInternal() {
    if (!isOpen())
        return;

    if (_chunks.empty()) {
        *_target = Blob().withDisplayPath(displayPath());
    } else if (_chunks.size() == 1) {
        // Single chunk: move the malloc'd memory directly into the Blob, no copy needed.
        *_target = Blob::fromMalloc(std::move(_chunks.front().data), _chunks.front().size - buffer().remaining())
            .withDisplayPath(displayPath());
    } else {
        *_target = materialize();
    }

    _target = nullptr;
    _chunks.clear();
}

Blob BlobOutputStream::materialize() {
    // Compute total used size. All chunks except the last are sealed to their used size.
    size_t total = 0;
    for (const Chunk &chunk : _chunks)
        total += chunk.size;
    total -= buffer().remaining();
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
