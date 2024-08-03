#pragma once

#include <cstdio>
#include <cassert>
#include <utility>
#include <memory>
#include <string_view>
#include <string>
#include <type_traits>

#include "FreeDeleter.h"

class InputStream;

/**
 * `Blob` is an abstraction that couples a contiguous memory region with the knowledge of how to deallocate it.
 *
 * Deallocation is type-erased (like it's done in `std::shared_ptr`), so you don't have to pass in deleter as
 * a template parameter.
 *
 * For debug and error reporting purposes, `Blob` also stores a string describing the data source.
 */
class Blob final {
 public:
    Blob() {}

    Blob(const Blob &) = delete; // Blobs are non-copyable.

    Blob(Blob &&other) {
        swap(*this, other);
    }

    ~Blob() = default;

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
    [[nodiscard]] Blob subBlob(size_t offset, size_t size = -1) const;

    /**
     * @param data                      Pointer to a `malloc`-allocated memory region.
     * @param size                      Size of the memory region.
     * @return                          Blob that takes ownership of the provided memory region.
     */
    [[nodiscard]] static Blob fromMalloc(const void *data, size_t size);
    [[nodiscard]] static Blob fromMalloc(std::unique_ptr<void, FreeDeleter> data, size_t size);

    /**
     * @param path                      Path to a file.
     * @return                          Blob that wraps the memory mapping of the provided file, with display path
     *                                  set to `path`.
     * @throws std::runtime_error       If file doesn't exist or on some other OS error.
     */
    [[nodiscard]] static Blob fromFile(std::string_view path);

    /**
     * @param string                    String to create a blob from.
     * @return                          Blob that wraps the provided string.
     */
    [[nodiscard]] static Blob fromString(std::string string);

    /**
     * @param data                      Memory region pointer.
     * @param size                      Memory region size.
     * @return                          Blob that owns a copy of the provided memory region.
     */
    [[nodiscard]] static Blob copy(const void *data, size_t size); // NOLINT: this is not std::copy.

    /**
     * @param other                     Blob to copy.
     * @return                          Copy of the provided `Blob` in `malloc`-allocated memory.
     */
    [[nodiscard]] static Blob copy(const Blob &other); // NOLINT: this is not std::copy.

    /**
     * @param data                      Memory region pointer.
     * @param size                      Memory region size.
     * @return                          Non-owning blob view into the provided memory region.
     */
    [[nodiscard]] static Blob view(const void *data, size_t size);

    /**
     * @param data                      Memory region
     * @return                          Non-owning blob view into the provided memory region.
     */
    [[nodiscard]] static Blob view(std::string_view data);

    /**
     * @param file                      File to read from.
     * @param size                      Number of bytes to read.
     * @return                          Blob that owns the data that was read from the provided file.
     * @throws Exception                If the provided number of bytes couldn't be read.
     */
    [[nodiscard]] static Blob read(FILE *file, size_t size);

    /**
     * @param stream                    Stream to read from.
     * @param size                      Number of bytes to read.
     * @return                          Blob that owns the data that was read from the provided file, with display path
     *                                  set to the display path of the passed stream.
     * @throws Exception                If the provided number of bytes couldn't be read.
     */
    [[nodiscard]] static Blob read(InputStream *stream, size_t size);

    /**
     * @param l                         First blob.
     * @param r                         Second blob.
     * @return                          Newly allocated blob that contains a concatenation of the data of the two
     *                                  provided blobs.
     */
    [[nodiscard]] static Blob concat(const Blob &l, const Blob &r);

    /**
     * @param other                     Blob to share.
     * @return                          Blob that shares the memory ownership with `other`.
     */
    [[nodiscard]] static Blob share(const Blob &other);

    friend void swap(Blob &l, Blob &r) {
        using std::swap;
        swap(l._data, r._data);
        swap(l._size, r._size);
        swap(l._state, r._state);
        swap(l._displayPath, r._displayPath);
    }

    [[nodiscard]] size_t size() const {
        return _size;
    }

    [[nodiscard]] bool empty() const {
        return _size == 0;
    }

    [[nodiscard]] const void *data() const {
        return _data;
    }

    [[nodiscard]] bool operator!() const {
        return empty();
    }

    [[nodiscard]] explicit operator bool() const {
        return !empty();
    }

    [[nodiscard]] std::string_view string_view() const {
        return {static_cast<const char *>(_data), _size};
    }

    [[nodiscard]] const std::string &displayPath() const {
        return _displayPath;
    }

    Blob withDisplayPath(std::string_view displayPath) {
        _displayPath = displayPath;
        return std::move(*this);
    }

 private:
    const void *_data = nullptr;
    size_t _size = 0;
    std::shared_ptr<void> _state;
    std::string _displayPath;
};
