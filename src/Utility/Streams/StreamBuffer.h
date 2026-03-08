#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>
#include <type_traits>

/**
 * Three-pointer buffer used by `InputStream` and `OutputStream`.
 *
 * Maintains the invariant that all three pointers are non-null and sorted: `start <= pos <= end`.
 *
 * A default-constructed buffer points to a valid empty sentinel. This is mainly needed so that we don't have to jump
 * through hoops when calling `memcpy` (calling it with `nullptr` is UB even if size is 0).
 *
 * @tparam T                            `char` for writable buffers, `const char` for read-only buffers.
 */
template<class T> requires (std::is_same_v<std::remove_const_t<T>, char>)
class StreamBuffer {
 public:
    StreamBuffer() {
        reset(nullptr, nullptr, nullptr);
    }

    StreamBuffer(T *start, T *pos, T *end) {
        reset(start, pos, end);
    }

    void reset(T *newStart, T *newPos, T *newEnd) {
        assert(newStart <= newPos && newPos <= newEnd);

        if (!newStart) {
            assert(!newPos && !newEnd);
            static std::remove_const_t<T> emptyBuffer = {};
            newStart = newPos = newEnd = &emptyBuffer;
        }

        _start = newStart;
        _pos = newPos;
        _end = newEnd;
    }

    [[nodiscard]] T *start() const { return _start; }
    [[nodiscard]] T *pos() const { return _pos; }
    [[nodiscard]] T *end() const { return _end; }
    [[nodiscard]] size_t used() const { return static_cast<size_t>(_pos - _start); }
    [[nodiscard]] size_t remaining() const { return static_cast<size_t>(_end - _pos); }

    void commit() {
        _start = _pos;
    }

    size_t read(void *dst, size_t size) {
        assert(dst);
        assert(size <= remaining());

        memcpy(dst, _pos, size);
        _pos += size;
        return size;
    }

    size_t read(std::string *dst, size_t size) {
        assert(dst);
        assert(size <= remaining());

        dst->append(_pos, size);
        _pos += size;
        return size;
    }

    size_t skip(size_t size) {
        assert(size <= remaining());

        _pos += size;
        return size;
    }

    size_t write(const void *src, size_t size) requires (!std::is_const_v<T>) {
        assert(src);
        assert(size <= remaining());

        memcpy(_pos, src, size);
        _pos += size;
        return size;
    }

 private:
    T *_start;
    T *_pos;
    T *_end;
};
