#pragma once

#include <concepts>
#include <type_traits>

/**
 * Concepts checking that `serialize()` is callable with the provided arguments.
 */
template<class Src, class Dst, class... Tag>
concept BinarySerializable = requires(const Src &src, Dst *dst, Tag... tag) {
    { serialize(src, dst, tag...) } -> std::same_as<void>;
}; // NOLINT

/**
 * Concept checking that `deserialize()` is callable with the provided arguments.
 */
template<class Src, class Dst, class... Tag>
concept BinaryDeserializable = requires(Src &src, Dst *dst, Tag... tag) {
    { deserialize(src, dst, tag...) } -> std::same_as<void>;
}; // NOLINT

template<class T>
struct is_proxy_binary_serialization_source : std::false_type {};

template<class T>
constexpr bool is_proxy_binary_serialization_source_v = is_proxy_binary_serialization_source<T>::value;

template<class T>
struct is_proxy_binary_serialization_target : std::false_type {};

template<class T>
constexpr bool is_proxy_binary_serialization_target_v = is_proxy_binary_serialization_target<T>::value;

/**
 * Concept checking that the provided type is not a proxy serialization source type.
 */
template<class Src>
concept RegularBinarySource = !is_proxy_binary_serialization_source_v<std::remove_cvref_t<Src>>;

/**
 * Concept checking that the provided type is not a proxy serialization target type.
 */
template<class Dst>
concept RegularBinaryTarget = !is_proxy_binary_serialization_target_v<std::remove_cvref_t<Dst>>;
