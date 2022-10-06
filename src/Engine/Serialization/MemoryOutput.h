#pragma once

#include <span>
#include <vector>

#include <Utility/Blob.h>

class MemoryOutput {
 public:
    MemoryOutput() {}

    MemoryOutput(const MemoryOutput &) = delete;

    ~MemoryOutput() {
        Reset();
    }

    void Reset();

    Blob Finish();

    template<class T>
    void WriteRaw(const T *src) {
        memcpy(Allocate(sizeof(T)), src, sizeof(T));
    }

    template<class T>
    void WriteRawArray(const T *src, size_t size) {
        memcpy(Allocate(sizeof(T) * size), src, sizeof(T) * size);
    }

    template<class T>
    void WriteVector(const std::vector<T> &src) {
        WriteVectorInternal(src);
    }

    template<class LegacyT, class T>
    void WriteLegacyVector(const std::vector<T> &src) {
        WriteVectorInternal<LegacyT>(src);
    }

    template<class T>
    void WriteSizedVector(const std::vector<T> &src) {
        WriteVectorInternal(src, false);
    }

    template<class LegacyT, class T>
    void ReadSizedLegacyVector(const std::vector<T> &src) {
        WriteVectorInternal<LegacyT>(src, false);
    }

 private:
    template<class LegacyT = void, class T>
    void WriteVectorInternal(const std::vector<T> &src, bool writeSize = true) {
        if (writeSize) {
            uint32_t size = src.size();
            WriteRaw(&size);
        }

        constexpr bool isLegacyMode = !std::is_same_v<LegacyT, void>;
        if constexpr (isLegacyMode) {
            LegacyT tmp;
            for (const T &element : src) {
                tmp.Serialize(&element);
                WriteRaw(&tmp);
            }
        } else {
            WriteRawArray(src.data(), src.size());
        }
    }

    void *Allocate(size_t size);

 private:
    size_t pos_ = 0;
    std::vector<std::span<std::byte>> spans_;
};
