#include "BlobInputStream.h"

#include <cassert>
#include <cstring>
#include <algorithm>

BlobInputStream::BlobInputStream(const Blob &blob) {
    _blob = &blob;
    _pos = static_cast<const char *>(blob.data());
    _end = _pos + blob.size();
}

size_t BlobInputStream::read(void *data, size_t size) {
    assert(_pos);

    size_t result = std::min(size, static_cast<size_t>(_end - _pos));

    memcpy(data, _pos, result);
    _pos += result;
    return result;
}

size_t BlobInputStream::skip(size_t size) {
    assert(_pos);

    size_t result = std::min(size, static_cast<size_t>(_end - _pos));
    _pos += result;
    return result;
}

void BlobInputStream::close() {
    _blob = nullptr;
    _pos = nullptr;
    _end = nullptr;
}

Blob BlobInputStream::tail() const {
    assert(_pos);

    size_t offset = _pos - static_cast<const char *>(_blob->data());
    return _blob->subBlob(offset);
}
