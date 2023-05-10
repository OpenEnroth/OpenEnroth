#include "Blob.h"

#include <string>
#include <filesystem>

#include <mio/mmap.hpp>

#include "Utility/Streams/FileInputStream.h"
#include "Utility/Exception.h"

#include "FreeDeleter.h"

class NonOwningBlobHandler : public BlobHandler {
 public:
    virtual void destroy(Blob &self) override {
        assert(self.handler() == this);
    }

    virtual Blob share(Blob &self) override {
        assert(self.handler() == this);
        return Blob(self.data(), self.size(), this);
    }
};

class SharedBlobHandler : public BlobHandler {
 public:
    explicit SharedBlobHandler(std::shared_ptr<Blob> base): _base(std::move(base)) {}

    virtual void destroy(Blob &self) override {
        assert(self.handler() == this);
        delete this;
    }

    virtual Blob share(Blob &self) override {
        assert(self.handler() == this);
        return Blob(self.data(), self.size(), new SharedBlobHandler(_base));
    }

 private:
    std::shared_ptr<Blob> _base;
};

class SharableBlobHandler : public BlobHandler {
 public:
    virtual Blob share(Blob &self) override {
        assert(self.handler() == this);

        std::shared_ptr<Blob> base = std::make_shared<Blob>(std::move(self));
        self = Blob(base->data(), base->size(), new SharedBlobHandler(base));
        return Blob(base->data(), base->size(), new SharedBlobHandler(base));
    }
};

class FreeBlobHandler : public SharableBlobHandler {
 public:
    virtual void destroy(Blob &self) override {
        assert(self.handler() == this);
        free(const_cast<void *>(self.data()));
        // Note that we don't call `delete this` here.
    }
};

class MemoryMapBlobHandler : public SharableBlobHandler {
 public:
    explicit MemoryMapBlobHandler(mio::mmap_source mmap) : _mmap(std::move(mmap)) {}

    virtual void destroy(Blob &self) override {
        assert(self.handler() == this);
        delete this;
    }

 private:
    mio::mmap_source _mmap;
};

class StringBlobHandler : public SharableBlobHandler {
 public:
    explicit StringBlobHandler(std::string string) : _string(std::move(string)) {}

    virtual void destroy(Blob &self) override {
        assert(self.handler() == this);
        delete this;
    }

    std::string_view view() const {
        return _string;
    }

 private:
    std::string _string;
};

constinit FreeBlobHandler staticFreeBlobHandler = {};
constinit NonOwningBlobHandler staticNonOwningBlobHandler = {};

Blob Blob::subBlob(size_t offset, size_t size) {
    if (!_handler || offset >= _size || size == 0)
        return Blob();

    Blob result = _handler->share(*this);
    assert(result._data == _data && result._size == _size); // Just a sanity check.
    result._size = std::min(size, _size - offset);
    result._data = static_cast<const char *>(_data) + offset;
    return result;
}

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

Blob Blob::fromString(std::string string) {
    std::unique_ptr<StringBlobHandler> handler = std::make_unique<StringBlobHandler>(std::move(string));
    std::string_view view = handler->view();
    return Blob(view.data(), view.size(), handler.release());
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
