#include "Blob.h"

#include <string>
#include <filesystem>

#include <mio/mmap.hpp>

#include "Utility/Streams/FileInputStream.h"
#include "Utility/Exception.h"

#include "FreeDeleter.h"

class NonOwningBlobHandler : public BlobHandler {
 public:
    virtual void destroy(const void *, size_t) override {}
};

class FreeBlobHandler : public BlobHandler {
 public:
    virtual void destroy(const void *data, size_t) override {
        free(const_cast<void *>(data));
        // Note that we don't call `delete this` here.
    }
};

class MemoryMapBlobHandler : public BlobHandler {
 public:
    explicit MemoryMapBlobHandler(mio::mmap_source mmap) : _mmap(std::move(mmap)) {}

    virtual void destroy(const void *data, size_t) override {
        delete this;
    }

 private:
    mio::mmap_source _mmap;
};

constinit FreeBlobHandler staticFreeBlobHandler = {};
constinit NonOwningBlobHandler staticNonOwningBlobHandler = {};

Blob Blob::fromMalloc(const void *data, size_t size) {
    return Blob(data, size, &staticFreeBlobHandler);
}

Blob Blob::fromFile(std::string_view path) {
    std::string cpath(path);
    mio::mmap_source mmap(cpath); // Throws std::system_error.
    const void *data = mmap.data();
    size_t size = mmap.size();
    return Blob(data, size, new MemoryMapBlobHandler(std::move(mmap)));
}

Blob Blob::copy(const void *data, size_t size) { // NOLINT: this is not std::copy
    std::unique_ptr<void, FreeDeleter> memory(malloc(size)); // We don't handle allocation failures.

    memcpy(memory.get(), data, size);

    return Blob(memory.release(), size, &staticFreeBlobHandler);
}

Blob Blob::view(const void *data, size_t size) {
    return Blob(data, size, &staticNonOwningBlobHandler);
}

Blob Blob::read(FILE *file, size_t size) {
    if (size == 0)
        return Blob();

    std::unique_ptr<void, FreeDeleter> memory(malloc(size));

    size_t read = fread(memory.get(), size, 1, file);
    if (read != 1)
        throw Exception("Failed to read {} bytes from file", size);

    return Blob(memory.release(), size, &staticFreeBlobHandler);
}

Blob Blob::read(FileInputStream &file, size_t size) {
    if (size == 0)
        return Blob();

    std::unique_ptr<void, FreeDeleter> memory(malloc(size));
    file.readOrFail(memory.get(), size);
    return Blob(memory.release(), size, &staticFreeBlobHandler);
}

Blob Blob::concat(const Blob &l, const Blob &r) {
    std::unique_ptr<void, FreeDeleter> memory(malloc(l.size() + r.size()));

    memcpy(memory.get(), l.data(), l.size());
    memcpy(static_cast<char *>(memory.get()) + l.size(), r.data(), r.size());

    return Blob(memory.release(), l.size() + r.size(), &staticFreeBlobHandler);
}
