#pragma once

#include <concepts>
#include <iosfwd>
#include <string>
#include <string_view>

#include "SerializationFwd.h"
#include "StandardSerialization.h"

namespace detail {

struct StringSerializer {
    template<class T>
    std::string operator()(const T &value) const {
        std::string result;
        serialize(value, &result);
        return result;
    }
};

template<class T>
struct StringDeserializer {
    T operator()(std::string_view string) const {
        T result;
        deserialize(string, &result);
        return result;
    }
};

void printToStream(const std::string &string, std::ostream *stream); // This one is here so that we don't have to include <iostream>.

} // namespace detail

/**
 * Ranges-friendly serialization object.
 *
 * Can be used as `range | std::views::transform(ToString)`. Or directly, as `return ToString(some_integer);`.
 *
 * Under the hood it's just calling the `serialize` function using argument-dependent lookup.
 *
 * @see fromString
 */
constexpr detail::StringSerializer toString;

/**
 * Ranges-friendly deserialization object.
 *
 * Can be used as `range | std::views::transform(FromString<int>)`. Or directly, as `return FromString<int>(some_string)`.
 *
 * Under the hood it's just calling the `deserialize` function using argument-dependent lookup.
 *
 * @see toString
 */
template<class T>
constexpr detail::StringDeserializer<T> fromString;

/**
 * Concept for serializable types.
 */
template<class T>
concept Serializable = requires (T value, std::string str, std::string_view view) {
    { serialize(value, &str) } -> std::same_as<void>;
    { deserialize(view, &value) } -> std::same_as<void>;
    { trySerialize(value, &str) } -> std::same_as<bool>;
    { tryDeserialize(view, &value) } -> std::same_as<bool>;
}; // NOLINT: linter doesn't know anything about concepts.

// CLI11 support for `Serializable` types.
template<Serializable T>
inline bool lexical_cast(const std::string& src, T& dst) {
    return tryDeserialize(src, &dst);
}

// Google test printers support for `Serializable` types.
template<Serializable T> requires (!std::is_arithmetic_v<T>) // Don't override arithmetic type handling.
inline void PrintTo(const T &src, std::ostream *dst) {
    std::string tmp;
    serialize(src, &tmp);
    detail::printToStream(tmp, dst);
}
