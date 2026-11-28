#include "LegacyEncodingDetector.h"

#include <cstdint>

#include "Utility/String/Ascii.h"

TextEncoding detectLegacySaveEncoding(std::string_view names, TextEncoding fallback) {
    int letters = 0;
    int nonAsciiLetters = 0;

    for (char c : names) {
        if (static_cast<uint8_t>(c) >= 0x80) {
            letters++;
            nonAsciiLetters++;
        } else if (ascii::isUpper(c) || ascii::isLower(c)) {
            letters++;
        }
    }

    if (nonAsciiLetters == 0)
        return fallback;

    // Cyrillic names are all non-ASCII, western ones are ASCII with the odd accented letter.
    return nonAsciiLetters * 2 > letters ? ENCODING_WINDOWS_1251 : ENCODING_WINDOWS_1252;
}
