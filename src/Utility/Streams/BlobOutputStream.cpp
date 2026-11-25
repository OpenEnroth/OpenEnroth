#include "BlobOutputStream.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>

#include "Utility/Memory/FreeDeleter.h"

struct BlobOutputStream::Storage {
    char *data = nullptr;

    ~Storage() {
        free(data);
    }
};

BlobOutputStream::BlobOutputStream(Blob *target, std::string_view displayPath) {
    open(target, displayPath);
}

BlobOutputStream::~BlobOutputStream() {
    destroy();
}

void BlobOutputStream::open(Blob *target, std::string_view displayPath) {
    assert(target);
    _target = target;
    _storage.reset();
    base_type::open({}, displayPath);
}

void BlobOutputStream::_overflow(Buffer *buffer, const void *data, size_t size) {
    size_t used = buffer->used();
    size_t capacity = used + buffer->remaining();
    size_t newCapacity = std::max(used + size, capacity == 0 ? 1024 : capacity * 2);

    // Blobs published by `flush()` may still point into the old storage, so we allocate a new one instead of
    // `realloc`-ing. The copy is amortized O(1) per byte thanks to geometric growth.
    auto storage = std::make_shared<Storage>();
    storage->data = static_cast<char *>(malloc(newCapacity));
    if (used)
        memcpy(storage->data, buffer->start(), used);
    _storage = std::move(storage);

    *buffer = Buffer(_storage->data, _storage->data + used, _storage->data + newCapacity);
    buffer->write(data, size);
}

void BlobOutputStream::_flush(Buffer *buffer) {
    // No copy here: `flush()` is called after every log line by `StreamLogSink`, and copying the whole blob on each
    // Call made logging into a memory file system O(n^2). Bytes before `buffer->pos()` are never written
    // again, so the published blob can share the storage.
    *_target = publish(buffer);
}

void BlobOutputStream::_close(Buffer *buffer, bool canThrow) {
    assert(isOpen());

    *_target = Blob(); // Drop the last flushed blob so that we can check for unique ownership below.
    size_t size = buffer->used();
    char *shrunk = nullptr;
    if (size != 0 && _storage.use_count() == 1) {
        // Nobody else holds the storage - shrink it and hand it over, the shrinking `realloc` usually doesn't copy.
        shrunk = static_cast<char *>(realloc(_storage->data, size));
        if (shrunk)
            _storage->data = nullptr;
    }
    if (shrunk) {
        *_target = Blob::fromMalloc(std::unique_ptr<char, FreeDeleter>(shrunk), size).withDisplayPath(displayPath());
    } else {
        *_target = publish(buffer);
    }

    _target = nullptr;
    _storage.reset();

    base_type::_close(buffer, canThrow);
}

Blob BlobOutputStream::publish(Buffer *buffer) {
    if (buffer->used() == 0)
        return Blob().withDisplayPath(displayPath());
    return Blob::custom(_storage->data, buffer->used(), _storage).withDisplayPath(displayPath());
}
