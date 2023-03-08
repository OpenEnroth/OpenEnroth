#include "Blob.h"

#include <string>
#include <filesystem>

#include "Utility/Streams/FileInputStream.h"
#include "Utility/Exception.h"

class NonOwningBlobHandler : public BlobHandler {
 public:
    virtual void destroy(void *, size_t) override {}
};

class FreeBlobHandler : public BlobHandler {
 public:
    virtual void destroy(void *data, size_t) override {
        free(data);
        // Note that we don't call `delete this` here.
    }
};

constinit FreeBlobHandler staticFreeBlobHandler = {};
constinit NonOwningBlobHandler staticNonOwningBlobHandler = {};

Blob Blob::Allocate(size_t size) {
    Blob result;

    result.data_ = malloc(size); // We don't handle allocation failures.
    result.size_ = size;
    result.handler_ = &staticFreeBlobHandler;

    return result;
}

Blob Blob::Read(FILE *file, size_t size) {
    Blob result = Allocate(size);

    size_t read = fread(result.data_, result.size_, 1, file);
    if (read != 1)
        throw Exception("Failed to read {} bytes from file", size);

    return result;
}

Blob Blob::FromFile(std::string_view path) {
    size_t size = std::filesystem::file_size(path);
    Blob result = Allocate(size);

    FileInputStream stream(path);
    stream.ReadOrFail(result.data_, result.size_);
    return result;
}

Blob Blob::NonOwning(void *data, size_t size) {
    return Blob(data, size, &staticNonOwningBlobHandler);
}

Blob Blob::Concat(const Blob &l, const Blob &r) {
    Blob result = Allocate(l.size() + r.size());

    memcpy(result.data(), l.data(), l.size());
    memcpy(static_cast<char *>(result.data()) + l.size(), r.data(), r.size());

    return result;
}

std::shared_ptr<Blob> Blob::AllocateShared(size_t size) {
    return std::make_shared<Blob>(std::move(Allocate(size)));
}
