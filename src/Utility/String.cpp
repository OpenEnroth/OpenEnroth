#include "String.h"

#include <cstdarg>
#include <vector>
#include <algorithm>

static inline unsigned char AsciiToLower(unsigned char c) {
    return ((((c) >= 'A') && ((c) <= 'Z')) ? ((c) - 'A' + 'a') : (c));
}

static int AsciiCaseInsensitiveCompare(const char *l, const char *r, size_t size) {
    // There is no C api for ascii-only strnicmp, so we have to roll out our own.
    // The difference from the original strnicmp is that we don't check for null terminators.
    const unsigned char *ul = reinterpret_cast<const unsigned char *>(l);
    const unsigned char *ur = reinterpret_cast<const unsigned char *>(r);

    for (;size > 0; size--) {
        int result = static_cast<int>(AsciiToLower(*ul++)) - static_cast<int>(AsciiToLower(*ur++));
        if (result != 0)
            return result;
    }

    return 0;
}

static std::string_view ToCharStringView(std::u8string_view s) {
    return std::string_view(reinterpret_cast<const char *>(s.data()), s.size());
}

std::string StringFromInt(int value) {
    return StringPrintf("%d", value);
}

std::string StringPrintf(const char *fmt, ...) {
    int size = 1024;
    char *buffer = new char[size];

    va_list va;
    va_start(va, fmt);
    {
        int nsize = vsnprintf(buffer, size, fmt, va);
        if (size <= nsize) {  // fail delete buffer and try again
            delete[] buffer;
            buffer = new char[nsize + 1];  // +1 for \0
            nsize = vsnprintf(buffer, size, fmt, va);
        }
    }
    va_end(va);

    std::string ret(buffer);
    delete[] buffer;
    return ret;
}

std::vector<char *> Tokenize(char *input, const char separator) {
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

std::string ToLower(std::string_view text) {
    std::string result(text);
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string ToUpper(std::string_view text) {
    std::string result(text);
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

bool iequals(std::string_view a, std::string_view b) {
    if (a.size() != b.size())
        return false;

    return AsciiCaseInsensitiveCompare(a.data(), b.data(), a.size()) == 0;
}

bool iless(std::string_view a, std::string_view b) {
    int result = AsciiCaseInsensitiveCompare(a.data(), b.data(), std::min(a.size(), b.size()));
    if (result < 0)
        return true;
    if (result > 0)
        return false;
    return a.size() < b.size();
}

bool iequalsAscii(std::u8string_view a, std::u8string_view b) {
    return iequals(ToCharStringView(a), ToCharStringView(b));
}

bool ilessAscii(std::u8string_view a, std::u8string_view b) {
    return iless(ToCharStringView(a), ToCharStringView(b));
}
