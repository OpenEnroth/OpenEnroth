#pragma once

#include <array>
#include <cassert>

template <class _Ty, size_t _Size>
class NZIArray : std::array<_Ty, _Size> {
 public:
    _Ty& ZerothIndex() { return std::array<_Ty, _Size>::operator[](0); }

    _Ty& operator[](size_t _Pos) {  // subscript nonmutable sequence
        assert(_Pos != 0 && "not allowed to access zeroth element");
        return std::array<_Ty, _Size>::operator[](_Pos);
    }

    const _Ty& operator[](size_t _Pos) const {  // subscript nonmutable sequence
        assert(_Pos != 0 && "not allowed to access zeroth element");
        return std::array<_Ty, _Size>::operator[](_Pos);
    }
};
