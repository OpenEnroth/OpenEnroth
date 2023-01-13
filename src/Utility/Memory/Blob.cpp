#include "Blob.h"

class FreeBlobHandler : public BlobHandler {
 public:
    virtual void destroy(void *data, size_t) override {
        free(data);
        // Note that we don't call `delete this` here.
    }
};

constinit FreeBlobHandler staticFreeBlobHandler = {};

Blob Blob::Allocate(size_t size) {
    Blob result;

    result.data_ = malloc(size); // We don't handle allocation failures.
    result.size_ = size;
    result.handler_ = &staticFreeBlobHandler;

    return result;
}

std::shared_ptr<Blob> Blob::AllocateShared(size_t size) {
    return std::make_shared<Blob>(std::move(Allocate(size)));
}
