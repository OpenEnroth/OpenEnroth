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
    const char *start = static_cast<const char *>(_blob.data());
    Buffer buffer;
    buffer.reset(start, start, start + _blob.size());
    base_type::open(buffer, _blob.displayPath());
}

void BlobInputStream::open(const Blob &blob) {
    open(Blob::share(blob));
}

Blob BlobInputStream::readAsBlob(size_t size) {
    assert(isOpen());

    size = std::min(size, buffer().remaining());
    Blob result = _blob.subBlob(buffer().used(), size);
    (void) skip(size);
    return result;
}

Blob BlobInputStream::readAsBlobOrFail(size_t size) {
    assert(isOpen());

    if (size > buffer().remaining())
        throw Exception("Failed to read the requested number of bytes from a blob stream '{}', requested {}, got {}", _blob.displayPath(), size, buffer().remaining());

    return readAsBlob(size);
}

Blob BlobInputStream::readAllAsBlob() {
    assert(isOpen());

    Blob result = _blob.subBlob(buffer().used());
    (void) skip(buffer().remaining());
    return result;
}

