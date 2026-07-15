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
    destroy();
}

void BlobOutputStream::open(Blob *target, std::string_view displayPath) {
    assert(target);
    _target = target;
    _scratchpad.reset();
    base_type::open({}, displayPath);
}

void BlobOutputStream::_overflow(Buffer *buffer, const void *data, size_t size) {
    size_t head = buffer->write(data, buffer->remaining());
    data = static_cast<const char *>(data) + head;
    size -= head;
    *buffer = _scratchpad.next(size);
    buffer->write(data, size);
}

void BlobOutputStream::_flush(Buffer *buffer) {
    size_t bytesTotal = position();
    if (bytesTotal == 0) {
        *_target = Blob().withDisplayPath(displayPath());
        return;
    }

    std::unique_ptr<char, FreeDeleter> result(static_cast<char *>(malloc(bytesTotal)));
    _scratchpad.materialize(result.get(), bytesTotal);
    *_target = Blob::fromMalloc(std::move(result), bytesTotal).withDisplayPath(displayPath());
}

void BlobOutputStream::_close(Buffer *buffer, bool canThrow) {
    assert(isOpen());

    *_target = _scratchpad.finish(position()).withDisplayPath(displayPath());
    _target = nullptr;

    base_type::_close(buffer, canThrow);
}
