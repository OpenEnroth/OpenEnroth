#include "BlobOutputStream.h"

#include <cassert>
#include <cstdlib>
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
    _chunks.reset();
    base_type::open({}, displayPath);
}

void BlobOutputStream::_overflow(const void *data, size_t size, Buffer *buffer) {
    size_t head = buffer->write(data, buffer->remaining());
    data = static_cast<const char *>(data) + head;
    size -= head;
    *buffer = _chunks.allocateChunk(size);
    buffer->write(data, size);
}

void BlobOutputStream::_flush(Buffer *buffer) {
    *_target = materialize();
}

void BlobOutputStream::_close(Buffer *buffer) {
    assert(isOpen());
    closeInternal();
    base_type::_close(buffer);
}

Blob BlobOutputStream::materialize() {
    size_t bytesTotal = position();
    if (bytesTotal == 0)
        return Blob().withDisplayPath(displayPath());

    std::unique_ptr<char, FreeDeleter> result(static_cast<char *>(malloc(bytesTotal)));
    _chunks.materialize(result.get(), bytesTotal);
    return Blob::fromMalloc(std::move(result), bytesTotal).withDisplayPath(displayPath());
}

void BlobOutputStream::closeInternal() {
    if (!isOpen())
        return;

    if (_chunks.chunkCount() == 1) {
        *_target = Blob::fromMalloc(_chunks.popChunk(), position()).withDisplayPath(displayPath());
    } else {
        *_target = materialize();
    }

    _target = nullptr;
    _chunks.reset();
}
