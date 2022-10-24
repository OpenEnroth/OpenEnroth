#pragma once

#include <array>
#include <cassert>

template <class T, size_t Size>
class NZIArray {
 public:
    T& operator[](size_t index) {
        assert(index != 0 && "indexing starts at 1");
        return array_[index - 1];
    }

    const T& operator[](size_t index) const {
        assert(index != 0 && "indexing starts at 1");
        return array_[index - 1];
    }

 private:
    std::array<T, Size> array_;
};
