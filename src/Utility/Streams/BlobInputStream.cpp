#include "BlobInputStream.h"

#include <cassert>
#include <algorithm>
#include <utility>

#include "Utility/Exception.h"

BlobInputStream::BlobInputStream(Blob &&blob) {
    open(std::move(blob));
}

BlobInputStream::BlobInputStream(const Blob &blob) {
    open(blob);
}

void BlobInputStream::open(Blob &&blob) {
    _blob = std::move(blob);
    auto *start = static_cast<const char *>(_blob.data());
    InputStream::open(start, start + _blob.size(), _blob.displayPath());
}

void BlobInputStream::open(const Blob &blob) {
    open(Blob::share(blob));
}

Blob BlobInputStream::readAsBlob(size_t size) {
    assert(isOpen());

    size = std::min(size, bufferRemaining());
    Blob result = _blob.subBlob(bufferPosition(), size);
    (void) skip(size);
    return result;
}

Blob BlobInputStream::readAsBlobOrFail(size_t size) {
    assert(isOpen());

    if (size > bufferRemaining())
        throw Exception("Failed to read the requested number of bytes from a blob stream '{}', requested {}, got {}", _blob.displayPath(), size, bufferRemaining());

    return readAsBlob(size);
}

Blob BlobInputStream::readAllAsBlob() {
    assert(isOpen());

    Blob result = _blob.subBlob(bufferPosition());
    (void) skip(bufferRemaining());
    return result;
}

size_t BlobInputStream::bufferPosition() const {
    return static_cast<size_t>(bufferPos() - static_cast<const char *>(_blob.data()));
}
