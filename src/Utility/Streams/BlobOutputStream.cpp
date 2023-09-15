#include "BlobOutputStream.h"

#include <cassert>
#include <utility>

BlobOutputStream::BlobOutputStream(Blob *target) : base_type(&Embedded::get()), _target(target) {
    assert(target);
}

BlobOutputStream::~BlobOutputStream() {
    close();
}

void BlobOutputStream::flush() {
    assert(_target); // Should be open.

    // Flushing does the only sane thing, which is just making a copy. Shouldn't really be necessary in any of the
    // possible use cases.
    *_target = Blob::fromString(Embedded::get());
}

void BlobOutputStream::close() {
    if (!_target)
        return;

    base_type::close();
    *_target = Blob::fromString(std::move(Embedded::get()));
    _target = nullptr;
}
