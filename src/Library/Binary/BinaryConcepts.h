#pragma once

#include <concepts>
#include <type_traits>

template<class T>
struct is_proxy_binary_source : std::false_type {};

template<class T>
struct is_proxy_binary_sink : std::false_type {};

template<class T>
struct is_proxy_binarizable : std::false_type {};

/**
 * Binary serialization provides a set of overloads, and when there's too many, a moment comes when they need to be
 * explicitly ordered. Consider the following overloads:
 * ```
 * template<class T, class... Tag>
 * void serialize(const T &src, Blob *dst, const Tag &...); // #1
 * template<class Src, class Dst, class Via>
 * void serialize(const Src &src, Dst *dst, ViaTag<Via>); // #2
 * ```
 *
 * And the following call:
 * ```
 * Blob blob;
 * serialize(SomeData{}, &blob, tags::via<SomeData_Bin>);
 * ```
 *
 * This call is ambiguous because the types provided match both overloads, with neither one being a better match.
 * Besides, the ambiguity is genuine here - the order really doesn't matter, so whether `#1` calls into `#2` or the
 * other way around, result stays the same. However, this is C++, so we need to:
 * 1. Pick the order we like.
 * 2. Guide the compiler into it.
 *
 * In this particular case it makes sense to disambiguate on `src` and `dst` first, and then process all the tags.
 * How to do it? We can introduce the notion of 'proxy' binary serialization types:
 * - Proxy source is a type that can be deserialized-from, and that simply replaces the `src` with another type
 *   and forwards to another `deserialize` overload.
 * - Proxy sink is a type that can be serialized-into, and that simply replaces the `dst` with another type
 *   and forwards to another `serialize` overload.
 * - Proxy binarizable types are basically the same thing, but for client types that are serialized into binary form /
 *   deserialized from binary form by forwarding the corresponding call to another type. Note that calling serialize /
 *   deserialize for members doesn't mean that a type is a proxy.
 *
 * Thus, proxy types stand in contrast to regular, or "leaf" types - types for which non-template overloads are
 * provided. All overloads that process tags require regular `src` and `dst` arguments, and this provides natural
 * ordering:
 * 1. First, all `src` / `dst` transformations are carried out.
 * 2. Then, tags are applied in order.
 *
 * To achieve this, the following concepts are provided:
 * - `RegularBinarySource` to denote a non-proxy binary source (e.g., an `InputStream`).
 * - `RegularBinarySink` to denote a non-proxy binary sink (e.g., an `OutputStream`).
 * - `RegularBinarizable` to denote a non-proxy client type to be serialized or deserialized.
 *
 * Then, for the types that actually are proxies, the following type traits are provided for specialization:
 * - `is_proxy_binary_source` for proxy binary sources.
 * - `is_proxy_binary_sink` for proxy binary sinks.
 * - `is_proxy_binarizable` for proxy client types.
 *
 * @see RegularBinarySink
 * @see RegularBinarizable
 */
template<class T>
concept RegularBinarySource = !is_proxy_binary_source<std::remove_cvref_t<T>>::value;

/**
 * @see RegularBinarySource
 */
template<class T>
concept RegularBinarySink = !is_proxy_binary_sink<std::remove_cvref_t<T>>::value;

/**
 * @see RegularBinarySource
 */
template<class T>
concept RegularBinarizable = !is_proxy_binarizable<std::remove_cvref_t<T>>::value;

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
