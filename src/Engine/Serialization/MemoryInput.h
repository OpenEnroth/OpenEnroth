#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <memory>
#include <utility>
#include <type_traits>

#include "Utility/Blob.h"

class MemoryInput {
 public:
    // TODO: gcc chokes if this is turned into enum class, retest with GCC 12.3+
    enum VectorStoreMode {
        Append,
        Overwrite
    };

    MemoryInput() {}

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
    void ReadVector(std::vector<T> *dst, VectorStoreMode mode = Overwrite) {
        ReadVectorInternal(dst, mode);
    }

    template<class LegacyT, class T>
    void ReadLegacyVector(std::vector<T> *dst, VectorStoreMode mode = Overwrite) {
        ReadVectorInternal<LegacyT>(dst, mode);
    }

    template<class T>
    void ReadSizedVector(std::vector<T> *dst, size_t size, VectorStoreMode mode = Overwrite) {
        ReadVectorInternal(dst, mode, size);
    }

    template<class LegacyT, class T>
    void ReadSizedLegacyVector(std::vector<T> *dst, size_t size, VectorStoreMode mode = Overwrite) {
        ReadVectorInternal<LegacyT>(dst, mode, size);
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
    void ReadVectorInternal(std::vector<T> *dst, VectorStoreMode mode, size_t explicitSize = static_cast<size_t>(-1)) {
        uint32_t size = explicitSize;
        if (explicitSize == static_cast<size_t>(-1))
            ReadRaw(&size);

        if (mode == Overwrite)
            dst->clear();
        dst->reserve(dst->size() + size);

        constexpr bool isLegacyMode = !std::is_same_v<LegacyT, void>;
        std::conditional_t<isLegacyMode, LegacyT, char> tmp;

        for (size_t i = 0; i < size; i++) {
            if constexpr (isLegacyMode) {
                ReadRaw(&tmp);
                tmp.Deserialize(&dst->emplace_back());
            } else {
                ReadRaw(&dst->emplace_back());
            }
        }
    }

 private:
    std::unique_ptr<Blob> ownedMemory_;
    const char *data_ = nullptr;
    const char *end_ = nullptr;
};
