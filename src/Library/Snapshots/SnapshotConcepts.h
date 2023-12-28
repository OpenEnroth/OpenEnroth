#pragma once

#include <type_traits>

template<class Container1, class Container2>
concept DifferentElementTypes = !std::is_same_v<typename Container1::value_type, typename Container2::value_type>;

template<class T>
concept RawAccessible = requires (T &mutableValue, const T &constValue) {
    raw(mutableValue);
    raw(constValue);
}; // NOLINT: we need this ;
