#pragma once

#include <cassert>
#include <utility>
#include <memory>
#include <string_view>

class Blob final {
 public:
    using deleter_type = void(*)(void *, size_t);

    Blob() {}

    Blob(void *data, size_t size, deleter_type deleter) {
        assert(data ? size > 0 && deleter : true);
        assert(!data ? size == 0 && !deleter : true);

        data_ = data;
        size_ = size;
        deleter_ = deleter;
    }

    Blob(const Blob &) = delete; // Blobs are non-copyable.

    Blob(Blob &&other) {
        swap(*this, other);
    }

    ~Blob() {
        if (deleter_)
            deleter_(data_, size_);
    }

    Blob &operator=(const Blob &) = delete; // Blobs are non-copyable.

    Blob &operator=(Blob &&other) {
        swap(*this, other);
        return *this;
    }

    static Blob Allocate(size_t size);
    static std::shared_ptr<Blob> AllocateShared(size_t size);

    friend void swap(Blob &l, Blob &r) {
        std::swap(l.data_, r.data_);
        std::swap(l.size_, r.size_);
        std::swap(l.deleter_, r.deleter_);
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
    deleter_type deleter_ = nullptr;
};
