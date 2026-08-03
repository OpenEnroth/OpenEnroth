#include "BlobInputStream.h"

#include <cassert>
#include <algorithm>
#include <utility>

BlobInputStream::BlobInputStream(Blob &&blob) {
    open(std::move(blob));
}

BlobInputStream::BlobInputStream(const Blob &blob) {
    open(blob);
}

void BlobInputStream::open(Blob &&blob) {
    _blob = std::move(blob);
    const char *start = static_cast<const char *>(_blob.data());
    base_type::open(Buffer(start, start, start + _blob.size()), _blob.size(), _blob.displayPath());
}

void BlobInputStream::open(const Blob &blob) {
    open(Blob::share(blob));
}

Blob BlobInputStream::readAsBlob(size_t size) {
    assert(isOpen());

    size_t bytes = skip(size);
    return _blob.subBlob(position() - bytes, bytes);
}

Blob BlobInputStream::readAsBlobOrFail(size_t size) {
    assert(isOpen());

    // TODO(captainurist): `size() - position()` underflows if `position()` ever runs past `size()`, and then this
    //                     check passes for any requested size. Needs a saturating `remaining()`.
    if (size > this->size() - position())
        throwReadError(size, this->size() - position());

    return readAsBlob(size);
}

void BlobInputStream::_close(bool canThrow) {
    // Note that the base class has to go first - it drops the buffer, which points into the blob that we're about to
    // release here.
    base_type::_close(canThrow);
    _blob = Blob(); // For a file-backed blob this is an mmap, no point in holding it until this object is destroyed.
}

Blob BlobInputStream::readAllAsBlob() {
    assert(isOpen());

    size_t bytes = skip(size());
    return _blob.subBlob(position() - bytes);
}
