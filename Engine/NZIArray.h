#pragma once
#include <array>
#include <assert.h>


template<class _Ty,
  size_t _Size>
class NZIArray : std::array<_Ty, _Size>
{
public:
  reference ZerothIndex()
  {
    return std::array<_Ty, _Size>::operator [](0);
  }

  reference operator[](size_type _Pos)
  {	// subscript nonmutable sequence
#if _ITERATOR_DEBUG_LEVEL == 2
    assert(_Pos != 0 && "not allowed to access zeroth element");

#elif _ITERATOR_DEBUG_LEVEL == 1
    _SCL_SECURE_VALIDATE_RANGE(_Pos != 0);
#endif /* _ITERATOR_DEBUG_LEVEL */

    __analysis_assume(_Pos != 0);

    return std::array<_Ty, _Size>::operator [](_Pos);
  }

  const_reference operator[](size_type _Pos) const
  {	// subscript nonmutable sequence
#if _ITERATOR_DEBUG_LEVEL == 2
    assert(_Pos != 0 && "not allowed to access zeroth element");

#elif _ITERATOR_DEBUG_LEVEL == 1
    _SCL_SECURE_VALIDATE_RANGE(_Pos != 0);
#endif /* _ITERATOR_DEBUG_LEVEL */

    __analysis_assume(_Pos != 0);

    return std::array<_Ty, _Size>::operator [](_Pos);
  }
};
