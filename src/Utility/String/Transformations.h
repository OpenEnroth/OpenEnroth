#pragma once

#include <string>
#include <string_view>

/**
 * @param str                           String to unquote.
 * @return                              String with leading & trailing double quotes removed, if any.
 * @offset 0x00452C30
 */
inline std::string_view removeQuotes(std::string_view str) {
    std::string_view result = str;
    if (result.starts_with('"'))
        result = result.substr(1);
    if (result.ends_with('"'))
        result = result.substr(0, result.size() - 1);
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
