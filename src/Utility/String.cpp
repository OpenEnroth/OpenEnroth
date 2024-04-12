#include "String.h"

#include <cassert>
#include <vector>
#include <algorithm>
#include <string>

#include "Format.h"

static inline unsigned char asciiToLower(unsigned char c) {
    return ((((c) >= 'A') && ((c) <= 'Z')) ? ((c) - 'A' + 'a') : (c));
}

static inline unsigned char asciiToUpper(unsigned char c) {
    return ((((c) >= 'a') && ((c) <= 'z')) ? ((c) - 'a' + 'A') : (c));
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
    std::transform(result.begin(), result.end(), result.begin(), &asciiToLower);
    return result;
}

std::string toUpper(std::string_view text) {
    std::string result(text);
    std::transform(result.begin(), result.end(), result.begin(), &asciiToUpper);
    return result;
}

bool noCaseStartsWith(std::string_view s, std::string_view prefix) {
    if (s.size() < prefix.size())
        return false;

    return asciiCaseInsensitiveCompare(s.data(), prefix.data(), prefix.size()) == 0;
}

bool noCaseEquals(std::string_view a, std::string_view b) {
    if (a.size() != b.size())
        return false;

    return asciiCaseInsensitiveCompare(a.data(), b.data(), a.size()) == 0;
}

bool noCaseLess(std::string_view a, std::string_view b) {
    int result = asciiCaseInsensitiveCompare(a.data(), b.data(), std::min(a.size(), b.size()));
    if (result < 0)
        return true;
    if (result > 0)
        return false;
    return a.size() < b.size();
}

bool noCaseEqualsAscii(std::u8string_view a, std::u8string_view b) {
    return noCaseEquals(toCharStringView(a), toCharStringView(b));
}

bool noCaseLessAscii(std::u8string_view a, std::u8string_view b) {
    return noCaseLess(toCharStringView(a), toCharStringView(b));
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

std::string replaceAll(std::string_view text, std::string_view what, std::string_view replacement) {
    assert(!what.empty());
    std::string result;

    size_t pos = 0;
    while(true) {
        size_t next = text.find(what, pos);
        result.append(text.substr(pos, next == std::string_view::npos ? std::string_view::npos : next - pos));
        if (next == std::string_view::npos)
            break;

        result.append(replacement);
        pos = next + what.size();
    }

    return result;
}

void splitString(std::string_view s, char sep, std::vector<std::string_view> *result) {
    result->clear();
    if(s.empty())
        return;

    result->reserve(16);
    const char *pos = s.data();
    const char *end = s.data() + s.size();
    while (pos != end + 1) {
        const char *next = std::find(pos, end, sep);

        result->emplace_back(pos, next);
        pos = next + 1;
    }
}
