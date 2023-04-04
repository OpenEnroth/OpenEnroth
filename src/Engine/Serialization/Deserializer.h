#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "Utility/Memory/Blob.h"
#include "Utility/Streams/InputStream.h"
#include "Utility/Streams/MemoryInputStream.h"
#include "Utility/Embedded.h"
#include "Utility/Exception.h"

class Deserializer {
 public:
    // TODO: gcc chokes if this is turned into enum class, retest with GCC 12.3+
    enum VectorStoreMode {
        Append,
        Overwrite
    };

    explicit Deserializer(InputStream *inputStream) {
        Reset(inputStream);
    }

    void Reset(InputStream *inputStream) {
        assert(inputStream);

        inputStream_ = inputStream;
    }

    void ReadBytes(void *dst, size_t size) {
        size_t bytesRead = inputStream_->read(dst, size);
        if (bytesRead < size)
            throw Exception("Deserialization failed, no more data in stream.");
    }

    void SkipBytes(size_t size) {
        size_t skippedBytes = inputStream_->skip(size);
        if (skippedBytes != size)
            throw Exception("Deserialization failed, no more data in stream.");
    }

    template<class T>
    void ReadRaw(T *dst) {
        ReadBytes(dst, sizeof(T));
    }

    template<class T>
    void ReadRawArray(T *dst, size_t size) {
        ReadBytes(dst, sizeof(T) * size);
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
                Deserialize(tmp, &dst->emplace_back());
            } else {
                ReadRaw(&dst->emplace_back());
            }
        }
    }

 private:
    InputStream *inputStream_ = nullptr;
};


class BlobDeserializer: private Embedded<MemoryInputStream>, public Deserializer {
    using StreamBase = Embedded<MemoryInputStream>;
 public:
    BlobDeserializer() : Deserializer(&StreamBase::get()) {}

    explicit BlobDeserializer(const Blob &blob) : BlobDeserializer() {
        Reset(blob);
    }

    explicit BlobDeserializer(Blob &&blob) : BlobDeserializer() {
        Reset(std::move(blob));
    }

    void Reset(const Blob &blob) {
        StreamBase::get().reset(blob.data(), blob.size());
    }

    void Reset(Blob &&blob) {
        blob_ = std::make_unique<Blob>(std::move(blob));
        Reset(*blob_);
    }

 private:
    std::unique_ptr<Blob> blob_;
};
