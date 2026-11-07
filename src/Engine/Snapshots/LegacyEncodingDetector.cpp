#include "LegacyEncodingDetector.h"

#include <cstdint>

#include "Utility/String/Ascii.h"

TextEncoding detectLegacySaveEncoding(std::string_view names) {
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

    // Cyrillic names are all non-ASCII, western ones are ASCII with the odd accented letter, so anything above a
    // handful of non-ASCII letters means Cyrillic. Names with no letters at all end up as windows-1252, which is
    // fine - it agrees with windows-1251 on everything they can possibly contain.
    return nonAsciiLetters * 2 > letters ? ENCODING_WINDOWS_1251 : ENCODING_WINDOWS_1252;
}
