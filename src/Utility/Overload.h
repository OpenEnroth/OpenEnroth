#pragma once

#include <type_traits>
#include <utility>

namespace detail {
template<class... Functions>
struct Overloaded : Functions... {
    using Functions::operator()...;
};
} // namespace detail

/**
 * Combines several function objects into one that is overloaded on their parameters.
 *
 * Example usage:
 * ```
 * auto print = overload(
 *     [](int value) { fmt::println("int {}", value); },
 *     [](std::string_view value) { fmt::println("string {}", value); });
 * print(10); // Prints "int 10".
 * ```
 *
 * @param functions                     Function objects to combine, usually lambdas.
 * @return                              A function object that calls the one of `functions` that overload resolution
 *                                      picks for its arguments.
 */
template<class... Functions>
auto overload(Functions &&... functions) {
    return detail::Overloaded<std::remove_cvref_t<Functions>...>{std::forward<Functions>(functions)...};
}
