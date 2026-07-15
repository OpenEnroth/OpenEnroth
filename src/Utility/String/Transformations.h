#pragma once

#include <string>
#include <string_view>

/**
 * Unquotes a string using CSV-style quoting rules. If the string is double-quoted, strips the outer quotes and
 * un-doubles the inner ones (a literal quote inside a quoted string is escaped by doubling it, `""`). A string that
 * isn't double-quoted is returned as-is.
 *
 * @param str                           String to unquote.
 * @return                              Unquoted string.
 * @offset 0x00452C30
 */
inline std::string unquote(std::string_view str) {
    if (!str.starts_with('"'))
        return std::string(str); // Not a quoted field, the `""` escaping doesn't apply.

    str = str.substr(1);
    if (str.ends_with('"'))
        str = str.substr(0, str.size() - 1);

    std::string result;
    result.reserve(str.size());
    for (size_t i = 0; i < str.size(); i++) {
        result += str[i];
        if (str[i] == '"' && i + 1 < str.size() && str[i + 1] == '"')
            i++; // Doubled quote, emit only one.
    }
    return result;
}

inline std::string trimRemoveQuotes(std::string_view str) {
    while (str.starts_with(' ') || str.starts_with('"'))
        str = str.substr(1);
    while (str.ends_with(' ') || str.ends_with('"'))
        str = str.substr(0, str.size() - 1);
    return std::string(str);
}

inline std::string_view trim(std::string_view s) {
    size_t l = 0;
    size_t r = s.size();
    while (l < r && s[l] == ' ')
        l++;
    while (l < r && s[r - 1] == ' ')
        r--;
    return s.substr(l, r - l);
}

/**
 * `toHexDump("0123", 2)` is `"3132 3334"`.
 *
 * @param s                             String to convert.
 * @param groupSize                     Size of a single group, in bytes.
 * @return                              Hex dump of the provided string.
 */
std::string toHexDump(std::string_view s, size_t groupSize = 0);

std::string replaceAll(std::string_view text, std::string_view what, std::string_view replacement);

std::string replaceAll(std::string_view text, char what, char replacement);
