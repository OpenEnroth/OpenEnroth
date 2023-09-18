#include "String.h"

#include <cstdarg>
#include <vector>
#include <algorithm>

#include "Format.h"

static inline unsigned char asciiToLower(unsigned char c) {
    return ((((c) >= 'A') && ((c) <= 'Z')) ? ((c) - 'A' + 'a') : (c));
}

static int asciiCaseInsensitiveCompare(const char *l, const char *r, size_t size) {
    // There is no C api for ascii-only strnicmp, so we have to roll out our own.
    // The difference from the original strnicmp is that we don't check for null terminators.
    const unsigned char *ul = reinterpret_cast<const unsigned char *>(l);
    const unsigned char *ur = reinterpret_cast<const unsigned char *>(r);

    for (; size > 0; size--) {
        int result = static_cast<int>(asciiToLower(*ul++)) - static_cast<int>(asciiToLower(*ur++));
        if (result != 0)
            return result;
    }

    return 0;
}

static std::string_view toCharStringView(std::u8string_view s) {
    return std::string_view(reinterpret_cast<const char *>(s.data()), s.size());
}

std::vector<char *> tokenize(char *input, const char separator) {
    std::vector<char *> retVect;
    retVect.push_back(input);
    while (*input) {
        if (*input == separator) {
            *input = '\0';
            retVect.push_back(input + 1);
        }
        ++input;
    }
    return retVect;
}

std::string toLower(std::string_view text) {
    std::string result(text);
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string toUpper(std::string_view text) {
    std::string result(text);
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

bool istarts_with(std::string_view s, std::string_view prefix) {
    if (s.size() < prefix.size())
        return false;

    return asciiCaseInsensitiveCompare(s.data(), prefix.data(), prefix.size()) == 0;
}

bool iequals(std::string_view a, std::string_view b) {
    if (a.size() != b.size())
        return false;

    return asciiCaseInsensitiveCompare(a.data(), b.data(), a.size()) == 0;
}

bool iless(std::string_view a, std::string_view b) {
    int result = asciiCaseInsensitiveCompare(a.data(), b.data(), std::min(a.size(), b.size()));
    if (result < 0)
        return true;
    if (result > 0)
        return false;
    return a.size() < b.size();
}

bool iequalsAscii(std::u8string_view a, std::u8string_view b) {
    return iequals(toCharStringView(a), toCharStringView(b));
}

bool ilessAscii(std::u8string_view a, std::u8string_view b) {
    return iless(toCharStringView(a), toCharStringView(b));
}

std::string toPrintable(std::string_view s, char placeholder) {
    std::string result(s.size(), placeholder);
    for (size_t i = 0; i < s.size(); i++)
        if (s[i] >= '\x20' && s[i] <= '\x7E') // Ascii printable chars are in [0x20, 0x7E].
            result[i] = s[i];
    return result;
}

std::string toHexDump(std::string_view s, size_t groupSize) {
    std::string result;
    for (size_t i = 0; i < s.size(); i++) {
        if (groupSize && i && i % groupSize == 0)
            result += " ";
        result += fmt::format("{:02X}", static_cast<uint8_t>(s[i]));
    }
    return result;
}
