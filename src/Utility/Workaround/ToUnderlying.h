#pragma once

#include <version>
#include <utility>

/* TODO: this workaround is bizarrely still needed for latest Android's NDK (25.1.8937393) which is based on CLang 14.0.6
 * Bring it back for now and investigate why later. */
#ifndef __cpp_lib_to_underlying

namespace std {
template <class T>
[[nodiscard]] inline constexpr underlying_type_t<T> to_underlying(T value) noexcept {
    return static_cast<underlying_type_t<T>>(value);
}
} // namespace std
#endif
