#pragma once

#include <concepts>
#include <type_traits>

namespace detail {
template<class T>
struct is_span : std::false_type {};

template<class T, size_t N>
struct is_span<std::span<T, N>> : std::true_type {};
} // namespace detail

template<class T>
struct is_binary_serialization_proxy : std::false_type {};

template<class T>
constexpr bool is_binary_serialization_proxy_v = is_binary_serialization_proxy<T>::value;

/**
 * Concept checking that the provided type is not a proxy binary serialization type.
 */
template<class T>
concept NonBinaryProxy = !is_binary_serialization_proxy_v<std::remove_cvref_t<T>>;

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

/**
 * Concept that matches std::span. This is mainly needed so that overloads that take `std::span` won't accidentally
 * trigger user-defined conversions, and thus won't be callable with `std::vector` or other span-compatible types.
 */
template<class Container>
concept StdSpan = detail::is_span<Container>::value;
