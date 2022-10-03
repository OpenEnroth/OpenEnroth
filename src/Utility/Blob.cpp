#include "Blob.h"

static void free_deleter(void *data, size_t) {
    free(data);
}

Blob Blob::Allocate(size_t size) {
    Blob result;

    result.data_ = malloc(size); // We don't handle allocation failures.
    result.size_ = size;
    result.deleter_ = &free_deleter;

    return result;
}

std::shared_ptr<Blob> Blob::AllocateShared(size_t size) {
    return std::make_shared<Blob>(std::move(Allocate(size)));
}
