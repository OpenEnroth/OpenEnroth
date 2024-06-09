#include "BlobOutputStream.h"

#include <cassert>
#include <utility>

BlobOutputStream::BlobOutputStream() {}


BlobOutputStream::BlobOutputStream(Blob *target) : BlobOutputStream() {
    open(target);
}

BlobOutputStream::~BlobOutputStream() {
    closeInternal();
}

void BlobOutputStream::open(Blob *target) {
    assert(target);

    closeInternal();
    assert(Embedded::get().empty());

    _target = target;
    base_type::open(&Embedded::get());
}

void BlobOutputStream::flush() {
    assert(_target); // Should be open.

    // Flushing does the only sane thing, which is just making a copy. Shouldn't really be necessary in any of the
    // possible use cases.
    *_target = Blob::fromString(Embedded::get());
}

void BlobOutputStream::close() {
    closeInternal();
}

void BlobOutputStream::closeInternal() {
    if (!_target)
        return;

    base_type::close();
    *_target = Blob::fromString(std::move(Embedded::get()));
    _target = nullptr;
}
