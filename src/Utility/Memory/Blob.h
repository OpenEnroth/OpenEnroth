#pragma once

#include <cassert>
#include <cstdlib>
#include <array>
#include <utility>
#include <memory>
#include <string_view>
#include <string>

class FileInputStream;
class Blob;

class BlobHandler {
 public:
    virtual ~BlobHandler() = default;
    virtual void destroy(Blob &self) = 0;
    virtual Blob share(Blob &self) = 0;
};

/**
 * `Blob` is an abstraction that couples a contiguous memory region with the knowledge of how to deallocate it.
 *
 * Deallocation is type-erased (like it's done in `std::shared_ptr`), so you don't have to pass in deleter as
 * a template parameter.
 */
class Blob final {
 public:
    Blob() {}

    Blob(const void *data, size_t size, BlobHandler *handler) {
        assert(data ? size > 0 && handler : true);
        assert(!data ? size == 0 && !handler : true);

        _data = data;
        _size = size;
        _handler = handler;
    }

    Blob(const Blob &) = delete; // Blobs are non-copyable.

    Blob(Blob &&other) {
        swap(*this, other);
    }

    ~Blob() {
        if (_handler)
            _handler->destroy(*this);
    }

    Blob &operator=(const Blob &) = delete; // Blobs are non-copyable.

    Blob &operator=(Blob &&other) {
        swap(*this, other);
        return *this;
    }

    /**
     * Pretty much like `substr`, but for `Blob`s.
     *
     * Important difference from `substr` is that the underlying memory is not copied and is instead shared between
     * the two blobs. It will be freed when both are destroyed, so cases where a small subblob holds a 100Mb chunk
     * of memory are possible, be careful.
     *
     * Also, this function doesn't throw `std::out_of_range` if `offset` is larger than the blob's size, returning
     * an empty blob in this case instead.
     *
     * @param offset                    Offset in the blob.
     * @param size                      Size of the subblob.
     * @return                          Subblob that shares the ownership of the underlying memory with this blob.
     */
    Blob subBlob(size_t offset, size_t size = -1);

    /**
     * @param data                      Pointer to a `malloc`-allocated memory region.
     * @param size                      Size of the memory region.
     * @return                          Blob that takes ownership of the provided memory region.
     */
    static Blob fromMalloc(const void *data, size_t size);

    /**
     * @param path                      Path to a file.
     * @return                          Blob that wraps the memory mapping of the provided file.
     * @throws std::runtime_error       On error.
     */
    static Blob fromFile(std::string_view path);

    /**
     * @param string                    String to create a blob from.
     * @return                          Blob that wraps the provided string.
     */
    static Blob fromString(std::string string);

    /**
     * @param data                      Memory region pointer.
     * @param size                      Memory region size.
     * @return                          Blob that owns a copy of the provided memory region.
     */
    static Blob copy(const void *data, size_t size); // NOLINT: this is not std::copy

    /**
     * @param data                      Memory region pointer.
     * @param size                      Memory region size.
     * @return                          Non-owning blob view into the provided memory region.
     */
    static Blob view(const void *data, size_t size);

    /**
     * @param file                      File to read from.
     * @param size                      Number of bytes to read.
     * @return                          Blob that owns the data that was read from the provided file.
     * @throws Exception                If the provided number of bytes couldn't be read.
     */
    static Blob read(FILE *file, size_t size);

    /**
     * @param file                      File to read from.
     * @param size                      Number of bytes to read.
     * @return                          Blob that owns the data that was read from the provided file.
     * @throws Exception                If the provided number of bytes couldn't be read.
     */
    static Blob read(FileInputStream &file, size_t size);

    /**
     * @param l                         First blob.
     * @param r                         Second blob.
     * @return                          Newly allocated blob that contains a concatenation of the data of the two
     *                                  provided blobs.
     */
    static Blob concat(const Blob &l, const Blob &r);

    friend void swap(Blob &l, Blob &r) {
        std::swap(l._data, r._data);
        std::swap(l._size, r._size);
        std::swap(l._handler, r._handler);
    }

    size_t size() const {
        return _size;
    }

    bool empty() const {
        return _size == 0;
    }

    const void *data() const {
        return _data;
    }

    bool operator!() const {
        return empty();
    }

    explicit operator bool() const {
        return !empty();
    }

    std::string_view string_view() const {
        return std::string_view(static_cast<const char *>(_data), _size);
    }

    BlobHandler *handler() const {
        return _handler;
    }

 private:
    const void *_data = nullptr;
    size_t _size = 0;
    BlobHandler *_handler = nullptr;
};
