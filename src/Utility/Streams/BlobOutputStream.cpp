#include "BlobOutputStream.h"

#include <cassert>
#include <string>
#include <utility>

BlobOutputStream::BlobOutputStream(Blob *target, std::string_view displayPath) {
    open(target, displayPath);
}

BlobOutputStream::~BlobOutputStream() {
    closeInternal();
}

void BlobOutputStream::open(Blob *target, std::string_view displayPath) {
    assert(target);

    closeInternal();

    _target = target;
    _displayPath = displayPath;
}

void BlobOutputStream::write(const void *data, size_t size) {
    assert(_target);

    _buffer.append(static_cast<const char *>(data), size);
}

void BlobOutputStream::flush() {
    assert(_target);

    // Flushing copies the data. Use close() to move instead.
    *_target = Blob::fromString(_buffer).withDisplayPath(_displayPath);
}

void BlobOutputStream::close() {
    closeInternal();
}

std::string BlobOutputStream::displayPath() const {
    return _displayPath;
}

void BlobOutputStream::closeInternal() {
    if (!_target)
        return;

    *_target = Blob::fromString(std::move(_buffer)).withDisplayPath(_displayPath);
    _target = nullptr;
    _buffer = {};
    _displayPath = {};
}
