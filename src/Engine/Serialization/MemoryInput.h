#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <memory>
#include <utility>

#include "Utility/Blob.h"

class MemoryInput {
 public:
    MemoryInput(const void *data, size_t size) {
        Reset(data, size);
    }

    explicit MemoryInput(const Blob &blob) {
        Reset(blob);
    }

    explicit MemoryInput(Blob &&blob) {
        Reset(std::move(blob));
    }

    void Reset(const void *data, size_t size) {
        data_ = static_cast<const char *>(data);
        end_ = static_cast<const char *>(data) + size;
    }

    void Reset(const Blob &blob) {
        Reset(blob.data(), blob.size());
    }

    void Reset(Blob &&blob) {
        Reset(blob.data(), blob.size());
        ownedMemory_ = std::make_unique<Blob>(std::move(blob));
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
        ReadVectorInternal(dst);
    }

    template<class LegacyT, class T>
    void ReadLegacyVector(std::vector<T> *dst) {
        ReadVectorInternal<LegacyT>(dst);
    }

    template<class T>
    void ReadSizedVector(std::vector<T> *dst, size_t size) {
        ReadVectorInternal(dst, size);
    }

    template<class LegacyT, class T>
    void ReadSizedLegacyVector(std::vector<T> *dst, size_t size) {
        ReadVectorInternal<LegacyT>(dst, size);
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

 private:
    template<class LegacyT = void, class T>
    void ReadVectorInternal(std::vector<T> *dst, size_t explicitSize = static_cast<size_t>(-1)) {
        uint32_t size = explicitSize;
        if (explicitSize == static_cast<size_t>(-1))
            ReadRaw(&size);

        if constexpr (std::is_same_v<LegacyT, void>) {
            dst->resize(size);
            ReadRawArray(dst->data(), size);
        } else {
            std::vector<LegacyT> tmp;
            tmp.resize(size);
            ReadRawArray(tmp.data(), size);

            dst->resize(tmp.size());
            for(size_t i = 0; i < tmp.size(); i++)
                tmp[i].Deserialize(&(*dst)[i]);
        }
    }

 private:
    std::unique_ptr<Blob> ownedMemory_;
    const char *data_ = nullptr;
    const char *end_ = nullptr;
};
