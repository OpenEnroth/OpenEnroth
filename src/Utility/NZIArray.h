#pragma once

#include <array>
#include <cassert>

template <class T, size_t Size>
class NZIArray {
 public:
    T& ZerothIndex() {
        return array_[0];
    }

    T& operator[](size_t index) {
        assert(index != 0 && "not allowed to access zeroth element");
        return array_[index];
    }

    const T& operator[](size_t index) const {
        assert(index != 0 && "not allowed to access zeroth element");
        return array_[index];
    }

 private:
    std::array<T, Size> array_;
};
