#pragma once

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

} // namespace detail

/**
 * Ranges-friendly serialization object.
 *
 * Can be used as `range | std::views::transform(toString)`. Or directly, as `return toString(some_integer);`.
 *
 * Under the hood it's just calling the `serialize` function using argument-dependent lookup.
 *
 * @see fromString
 */
constexpr detail::StringSerializer toString;

/**
 * Ranges-friendly deserialization object.
 *
 * Can be used as `range | std::views::transform(fromString<int>)`. Or directly, as `return fromString<int>(some_string)`.
 *
 * Under the hood it's just calling the `deserialize` function using argument-dependent lookup.
 *
 * @see toString
 */
template<class T>
constexpr detail::StringDeserializer<T> fromString;
