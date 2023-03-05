#pragma once

#include <cassert>
#include <cstdlib>
#include <utility>
#include <memory>
#include <string_view>

class BlobHandler {
 public:
    virtual void destroy(void *data, size_t size) = 0;
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

    Blob(void *data, size_t size, BlobHandler *handler) {
        assert(data ? size > 0 && handler : true);
        assert(!data ? size == 0 && !handler : true);

        data_ = data;
        size_ = size;
        handler_ = handler;
    }

    Blob(const Blob &) = delete; // Blobs are non-copyable.

    Blob(Blob &&other) {
        swap(*this, other);
    }

    ~Blob() {
        if (handler_)
            handler_->destroy(data_, size_);
    }

    Blob &operator=(const Blob &) = delete; // Blobs are non-copyable.

    Blob &operator=(Blob &&other) {
        swap(*this, other);
        return *this;
    }

    static Blob Allocate(size_t size);
    static Blob Read(FILE *file, size_t size);
    static Blob FromFile(std::string_view path);
    static Blob NonOwning(void *data, size_t size);
    static Blob Concat(const Blob &l, const Blob &r);
    static std::shared_ptr<Blob> AllocateShared(size_t size);

    friend void swap(Blob &l, Blob &r) {
        std::swap(l.data_, r.data_);
        std::swap(l.size_, r.size_);
        std::swap(l.handler_, r.handler_);
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    void *data() {
        return data_;
    }

    const void *data() const {
        return data_;
    }

    bool operator!() const {
        return empty();
    }

    explicit operator bool() const {
        return !empty();
    }

    std::string_view string_view() const {
        return std::string_view(static_cast<const char *>(data_), size_);
    }

 private:
    void *data_ = nullptr;
    size_t size_ = 0;
    BlobHandler *handler_ = nullptr;
};
