#pragma once

#include <type_traits>

template<class Container1, class Container2>
concept DifferentElementTypes = !std::is_same_v<typename Container1::value_type, typename Container2::value_type>;
