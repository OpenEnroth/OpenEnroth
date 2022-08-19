#pragma once

#include <vector>
#include <string>
#include <stdexcept>

class MemoryInput {
public:
    MemoryInput(const void *data, size_t size) {
        Reset(data, size);
    }

    void Reset(const void *data, size_t size) {
        data_ = static_cast<const char *>(data);
        end_ = static_cast<const char *>(data) + size;
    }

    template<class T>
    void ReadRaw(T *dst) {
        memcpy(dst, MapBytes(sizeof(T)), sizeof(T));
    }

    template<class T>
    void ReadRawArray(T *dst, size_t size) {
        memcpy(dst, MapBytes(sizeof(T) * size), sizeof(T) * size);
    }

    template<class T>
    void ReadVector(std::vector<T> *dst) {
        uint32_t size;
        ReadRaw(&size);
        ReadSizedVector(dst, size);
    }

    template<class T>
    void ReadSizedVector(std::vector<T> *dst, size_t size) {
        dst->resize(size);
        ReadRawArray(dst->data(), size);
    }

    void ReadSizedString(std::string *dst, size_t size) {
        dst->resize(size, '\0');
        ReadRawArray(dst->data(), size);
        dst->resize(strlen(dst->data()));
    }

    void Skip(size_t size) {
        MapBytes(size);
    }

    const void *MapBytes(size_t size) {
        if (size > end_ - data_)
            throw std::runtime_error(""); // TODO

        const void *result = data_;
        data_ += size;
        return result;
    }

    // TODO: drop
    const char *Ptr() const {
        return data_;
    }

private:
    const char *data_ = nullptr;
    const char *end_ = nullptr;
};
