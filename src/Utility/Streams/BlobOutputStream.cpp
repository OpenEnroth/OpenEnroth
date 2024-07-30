#include "BlobOutputStream.h"

#include <cassert>
#include <string>
#include <utility>

BlobOutputStream::BlobOutputStream() {}


BlobOutputStream::BlobOutputStream(Blob *target, std::string_view displayPath) : BlobOutputStream() {
    open(target, displayPath);
}

BlobOutputStream::~BlobOutputStream() {
    closeInternal();
}

void BlobOutputStream::open(Blob *target, std::string_view displayPath) {
    assert(target);

    closeInternal();
    assert(Embedded::get().empty());

    _target = target;
    _displayPath = displayPath;
    base_type::open(&Embedded::get());
}

void BlobOutputStream::flush() {
    assert(_target); // Should be open.

    // Flushing does the only sane thing, which is just making a copy. Shouldn't really be necessary in any of the
    // possible use cases.
    *_target = Blob::fromString(Embedded::get()).withDisplayPath(_displayPath);
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

    base_type::close();
    *_target = Blob::fromString(std::move(Embedded::get()));
    _target = nullptr;
    _displayPath = {};
}
