#pragma once

#include <string>

namespace detail {
template<class T>
constexpr bool is_bounded_char_array_v = false;

template<std::size_t N>
constexpr bool is_bounded_char_array_v<char[N]> = true;
} // namespace detail

template<class T>
concept JoinableToString =
    std::is_same_v<std::remove_cvref_t<T>, char> ||
    std::is_same_v<std::remove_cvref_t<T>, std::string_view> ||
    std::is_same_v<std::remove_cvref_t<T>, std::string> ||
    std::is_same_v<std::remove_cvref_t<T>, const char *> ||
    detail::is_bounded_char_array_v<std::remove_cvref_t<T>>;

template<JoinableToString... Joinables> requires (sizeof...(Joinables) > 0)
std::string join(Joinables &&... joinables) {
    std::string result;
    int _[] = {
        ((result += joinables), 0)...
    };
    return result;
}

template<class Strings>
    requires JoinableToString<decltype(*std::declval<Strings>().begin())> && // We can use std::ranges::range_value_t, but I'd rather not bring in <ranges>
             (!JoinableToString<Strings>)
std::string join(Strings &&strings, char sep) {
    std::string result;

    auto pos = strings.begin();
    if (pos == strings.end())
        return result;
    result += *pos++;

    while (pos != strings.end()) {
        result += sep;
        result += *pos++;
    }

    return result;
}
