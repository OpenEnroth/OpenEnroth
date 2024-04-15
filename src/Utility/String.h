#pragma once

#include <cstring>
#include <string>
#include <vector>

std::string toLower(std::string_view text);
std::string toUpper(std::string_view text);
std::vector<char*> tokenize(char *input, const char separator);

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

inline std::string trimRemoveQuotes(std::string str) {
    while (str.length() > 0 && (str.at(0) == ' ' || str.at(0) == '"')) {
        str.erase(0, 1);
    }
    while (str.length() > 0 && (str.at(str.length() - 1) == ' ' || str.at(str.length() - 1) == '"')) {
        str.pop_back();
    }

    return str;
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

bool noCaseStartsWith(std::string_view s, std::string_view prefix);
bool noCaseEquals(std::string_view a, std::string_view b);
bool noCaseLess(std::string_view a, std::string_view b);
bool noCaseEqualsAscii(std::u8string_view a, std::u8string_view b);
bool noCaseLessAscii(std::u8string_view a, std::u8string_view b);

struct NoCaseLess {
    using is_transparent = void; // This is a transparent comparator.
    bool operator()(std::string_view a, std::string_view b) const {
        return noCaseLess(a, b);
    }
};

/**
 * @param s                             String to transform.
 * @param placeholder                   Character to replace all non-printable chars with.
 * @return                              Passed string, with all non-printable characters replaced with `placeholder`.
 */
std::string toPrintable(std::string_view s, char placeholder = ' ');

/**
 * `toHexDump("0123", 2)` is `"3132 3334"`.
 *
 * @param s                             String to convert.
 * @param groupSize                     Size of a single group, in bytes.
 * @return                              Hex dump of the provided string.
 */
std::string toHexDump(std::string_view s, size_t groupSize = 0);

std::string replaceAll(std::string_view text, std::string_view what, std::string_view replacement);

void splitString(std::string_view s, char sep, std::vector<std::string_view> *result);

inline std::vector<std::string_view> splitString(std::string_view s, char sep) {
    std::vector<std::string_view> result;
    splitString(s, sep, &result);
    return result;
}

inline std::vector<std::string_view> splitString(const char *s, char sep) {
    return splitString(std::string_view(s), sep);
}

std::vector<std::string_view> splitString(std::string &&s, char sep) = delete; // Don't dangle!
