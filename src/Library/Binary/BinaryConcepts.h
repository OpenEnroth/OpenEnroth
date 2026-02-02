#pragma once

#include <concepts>
#include <type_traits>

/**
 * Type trait for greedy tags. Greedy tags should be processed before any source/destination conversions. Examples
 * include `ViaTag`.
 *
 * Regular tags (like `ContextTag`) just pass through and don't need special ordering.
 */
template<class T>
struct is_greedy_tag : std::false_type {};

/**
 * Checks if the first type in a parameter pack satisfies a given type trait.
 *
 * Returns `false` for an empty pack, otherwise returns `Condition<First>::value` where `First` is the first type
 * in the pack (with cv-ref qualifiers removed).
 *
 * @tparam Condition                    Type trait to check, e.g. `is_greedy_tag`.
 * @tparam Args                         Parameter pack to check.
 */
template<template<class> class Condition, typename... Args>
constexpr bool starts_with_v = false;

template<template<class> class Condition, class First, class... Rest>
constexpr bool starts_with_v<Condition, First, Rest...> = Condition<std::remove_cvref_t<First>>::value;

/**
 * Concept for std::vector-like containers.
 */
template<class Container>
concept ResizableContiguousContainer = requires (Container &container, size_t newSize) {
    typename Container::value_type;
    { container.resize(newSize) } -> std::same_as<void>;
    { container.size() } -> std::same_as<size_t>;
    { container.data() } -> std::convertible_to<const typename Container::value_type *>;
}; // NOLINT

namespace detail {
template<class T>
struct is_span : std::false_type {};

template<class T, size_t N>
struct is_span<std::span<T, N>> : std::true_type {};
} // namespace detail

/**
 * Concept that matches std::span. This is mainly needed so that overloads that take `std::span` don't accidentally
 * trigger user-defined conversions, and thus aren't callable with `std::vector` or other span-compatible types.
 */
template<class T>
concept StdSpan = detail::is_span<T>::value;
