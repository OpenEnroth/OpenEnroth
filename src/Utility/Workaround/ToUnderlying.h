#pragma once

#include <version>
#include <utility>

// TODO: drop this once we switch to Visual Studio 2022 in gitlab ci
#ifndef __cpp_lib_to_underlying

namespace std {
    template <class T>
    [[nodiscard]] inline constexpr underlying_type_t<T> to_underlying(T value) noexcept {
        return static_cast<underlying_type_t<T>>(value);
    }
}
#endif
