#pragma once

#include <cassert>
#include <cstring>
#include <memory>
#include <span>
#include <type_traits>

#include "MemoryScratchpad.h"

/**
 * Pointer-stable owning byte storage. `store` copies the passed data into this buffer and returns a view into the
 * buffer that stays valid for as long as the buffer itself is alive - growing the buffer with subsequent `store`
 * calls doesn't invalidate the previously returned views.
 */
class MemoryBuffer {
 public:
    explicit MemoryBuffer(size_t maxChunkSize = 1024 * 1024) : _chunks(maxChunkSize) {}

    /**
     * Copies the passed data into this buffer.
     *
     * @param data                      Data to copy.
     * @return                          Stable view into this buffer containing a copy of `data`.
     */
    template<class T>
    [[nodiscard]] std::span<const T> store(std::span<const T> data) {
        static_assert(std::is_trivially_copyable_v<T>, "MemoryBuffer stores raw memory.");

        if (data.empty())
            return {};

        // Align the free space for `T`. If the data doesn't fit, over-allocate the next chunk so that it definitely
        // has enough space at the right alignment. Chunks are malloc-aligned, so the alignment handling matters only
        // for over-aligned types, and when types of different alignments share a buffer. `std::align` adjusts the
        // members in place on success, and leaves them untouched when the data doesn't fit.
        if (!std::align(alignof(T), data.size_bytes(), _pos, _size)) {
            std::span<char> chunk = _chunks.next(data.size_bytes() + alignof(T) - 1);
            _pos = chunk.data();
            _size = chunk.size();
            [[maybe_unused]] void *aligned = std::align(alignof(T), data.size_bytes(), _pos, _size);
            assert(aligned);
        }

        const T *result = static_cast<const T *>(memcpy(_pos, data.data(), data.size_bytes()));
        _pos = static_cast<char *>(_pos) + data.size_bytes();
        _size -= data.size_bytes();
        return {result, data.size()};
    }

 private:
    MemoryScratchpad _chunks;
    void *_pos = nullptr; // Start of the unused part of the last allocated chunk.
    size_t _size = 0; // Size of the unused part.
};
