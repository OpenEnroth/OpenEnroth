#include "Ascii.h"

#include <algorithm>
#include <string>

namespace ascii {

static int noCaseCompare(const char *l, const char *r, size_t size) {
    // There is no C api for ascii-only strnicmp, so we have to roll out our own.
    // The difference from the original strnicmp is that we don't check for null terminators.

    for (; size > 0; size--) {
        int result = static_cast<unsigned char>(toLower(*l++)) - static_cast<unsigned char>(toLower(*r++));
        if (result != 0)
            return result;
    }

    return 0;
}

std::string toLower(std::string_view text) {
    std::string result(text);
    std::transform(result.begin(), result.end(), result.begin(), [](char c) { return toLower(c); });
    return result;
}

std::string toUpper(std::string_view text) {
    std::string result(text);
    std::transform(result.begin(), result.end(), result.begin(), [](char c) { return toUpper(c); });
    return result;
}

bool noCaseStartsWith(std::string_view s, std::string_view prefix) {
    if (s.size() < prefix.size())
        return false;

    return noCaseCompare(s.data(), prefix.data(), prefix.size()) == 0;
}

bool noCaseEquals(std::string_view a, std::string_view b) {
    if (a.size() != b.size())
        return false;

    return noCaseCompare(a.data(), b.data(), a.size()) == 0;
}

bool noCaseLess(std::string_view a, std::string_view b) {
    int result = noCaseCompare(a.data(), b.data(), std::min(a.size(), b.size()));
    if (result < 0)
        return true;
    if (result > 0)
        return false;
    return a.size() < b.size();
}

std::string toPrintable(std::string_view s, char placeholder) {
    std::string result(s.size(), placeholder);
    for (size_t i = 0; i < s.size(); i++)
        if (s[i] >= '\x20' && s[i] <= '\x7E') // Ascii printable chars are in [0x20, 0x7E].
            result[i] = s[i];
    return result;
}

} // namespace ascii
