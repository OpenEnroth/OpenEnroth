#include "BlobInputStream.h"

#include <cassert>
#include <cstring>
#include <algorithm>
#include <string>
#include <utility>

#include "Utility/Exception.h"

BlobInputStream::BlobInputStream(Blob &&blob) {
    open(std::move(blob));
}

BlobInputStream::BlobInputStream(const Blob &blob) {
    open(blob);
}

void BlobInputStream::open(Blob &&blob) {
    // No need to call close() here, we're overwriting all the fields anyway.
    _blob = std::move(blob);
    _pos = static_cast<const char *>(_blob.data());
    _end = _pos + _blob.size();
}

void BlobInputStream::open(const Blob &blob) {
    open(Blob::share(blob));
}

size_t BlobInputStream::read(void *data, size_t size) {
    assert(_pos);

    size_t result = std::min(size, remaining());

    memcpy(data, _pos, result);
    _pos += result;
    return result;
}

size_t BlobInputStream::skip(size_t size) {
    assert(_pos);

    size_t result = std::min(size, remaining());
    _pos += result;
    return result;
}

void BlobInputStream::close() {
    _blob = Blob();
    _pos = nullptr;
    _end = nullptr;
}

std::string BlobInputStream::displayPath() const {
    return _blob.displayPath();
}

void BlobInputStream::seek(ssize_t pos) {
    assert(_pos);

    _pos = static_cast<const char *>(_blob.data()) + std::clamp<ssize_t>(pos, 0, size());
}

ssize_t BlobInputStream::position() const {
    assert(_pos);

    return _pos - static_cast<const char *>(_blob.data());
}

ssize_t BlobInputStream::size() const {
    assert(_pos);

    return _blob.size();
}

Blob BlobInputStream::tail() const {
    assert(_pos);

    return _blob.subBlob(position());
}

Blob BlobInputStream::readBlob(size_t size) {
    assert(_pos);

    size = std::min(size, remaining());
    Blob result = _blob.subBlob(position(), size);
    assert(result.size() == size);
    _pos += size;
    return result;
}

Blob BlobInputStream::readBlobOrFail(size_t size) {
    assert(_pos);

    if (size > remaining())
        throw Exception("Failed to read the requested number of bytes from a blob stream, requested {}, got {}", size, remaining());

    Blob result = _blob.subBlob(position(), size);
    assert(result.size() == size);
    _pos += size;
    return result;
}

size_t BlobInputStream::remaining() const {
    assert(_pos);

    return static_cast<size_t>(_end - _pos);
}
