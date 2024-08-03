#pragma once

#include <string>
#include <string_view>

namespace ascii {

inline char isLower(char c) {
    return c >= 'a' && c <= 'z';
}

inline char isUpper(char c) {
    return c >= 'A' && c <= 'Z';
}

inline char toLower(char c) {
    return isUpper(c) ? c - 'A' + 'a' : c;
}

inline char toUpper(char c) {
    return isLower(c) ? c - 'a' + 'A' : c;
}

std::string toLower(std::string_view text);
std::string toUpper(std::string_view text);

bool noCaseStartsWith(std::string_view s, std::string_view prefix);
bool noCaseEquals(std::string_view a, std::string_view b);
bool noCaseLess(std::string_view a, std::string_view b);

struct NoCaseLess {
    using is_transparent = void; // This is a transparent comparator.
    bool operator()(std::string_view a, std::string_view b) const {
        return noCaseLess(a, b);
    }
};

/**
 * @param s                             String to transform.
 * @param placeholder                   Character to replace all non-printable chars with.
 * @return                              Passed string, with all non-printable non-ascii characters replaced with
 *                                      `placeholder`.
 */
std::string toPrintable(std::string_view s, char placeholder = ' ');

} // namespace ascii
