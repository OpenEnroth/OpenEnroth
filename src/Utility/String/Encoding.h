#pragma once

#include <cassert>
#include <cstddef>
#include <string>
#include <string_view>

// TODO(captainurist): #cpp26 can we just use std::text_encoding?

enum class TextEncoding {
    ENCODING_BYTES = 0, // Not-a-text = passthrough encoding.

    ENCODING_ASCII, // std::text_encoding::ASCII.

    // Unicode.
    ENCODING_UTF8, // std::text_encoding::UTF8.
    ENCODING_UTF16_BE, // std::text_encoding::UTF16BE.
    ENCODING_UTF16_LE, // std::text_encoding::UTF16LE.
    ENCODING_UTF32_BE, // std::text_encoding::UTF32BE.
    ENCODING_UTF32_LE, // std::text_encoding::UTF32LE.

    // ISO-8859 family.
    ENCODING_ISO_8859_1, // std::text_encoding::ISOLatin1.
    ENCODING_ISO_8859_2, // std::text_encoding::ISOLatin2.
    ENCODING_ISO_8859_3, // std::text_encoding::ISOLatin3.
    ENCODING_ISO_8859_4, // std::text_encoding::ISOLatin4.
    ENCODING_ISO_8859_5, // std::text_encoding::ISOLatinCyrillic.
    ENCODING_ISO_8859_6, // std::text_encoding::ISOLatinArabic.
    ENCODING_ISO_8859_7, // std::text_encoding::ISOLatinGreek.
    ENCODING_ISO_8859_8, // std::text_encoding::ISOLatinHebrew.
    ENCODING_ISO_8859_10, // std::text_encoding::ISOLatin6.
    ENCODING_ISO_8859_13, // std::text_encoding::ISO885913.
    ENCODING_ISO_8859_15, // std::text_encoding::ISO885915.
    ENCODING_ISO_8859_16, // std::text_encoding::ISO885916.

    // Windows codepages.
    ENCODING_WINDOWS_1251, // std::text_encoding::windows1251.
    ENCODING_WINDOWS_1252, // std::text_encoding::windows1252.
    ENCODING_WINDOWS_1253, // std::text_encoding::windows1253.
    ENCODING_WINDOWS_1255, // std::text_encoding::windows1255.
    ENCODING_WINDOWS_1256, // std::text_encoding::windows1256.
    ENCODING_WINDOWS_1257, // std::text_encoding::windows1257.
    ENCODING_WINDOWS_1258, // std::text_encoding::windows1258.

    // IBM codepages.
    ENCODING_IBM865, // std::text_encoding::IBM865.
    ENCODING_IBM866, // std::text_encoding::IBM866.

    // CJK encodings.
    ENCODING_BIG5, // std::text_encoding::Big5.
    ENCODING_EUC_KR, // std::text_encoding::EUCKR.
    ENCODING_GB18030, // std::text_encoding::GB18030.
    ENCODING_SHIFT_JIS, // std::text_encoding::ShiftJIS.

    // Other encodings.
    ENCODING_KOI8_R, // std::text_encoding::KOI8R.

    ENCODING_FIRST = ENCODING_ASCII,
    ENCODING_LAST = ENCODING_KOI8_R
};
using enum TextEncoding;

namespace txt {

/**
 * Convert a wide string to UTF-8.
 *
 * On Windows, the wide string is assumed to be UTF-16. On other platforms, it's assumed to be UTF-32.
 * Invalid sequences are replaced with the Unicode replacement character (U+FFFD).
 *
 * @param wstr                          Wide string to convert.
 * @return                              UTF-8 encoded string.
 */
std::string wideToUtf8(std::wstring_view wstr);

/**
 * Convert a UTF-8 string to a wide string.
 *
 * On Windows, the output is UTF-16. On other platforms, the output is UTF-32.
 * Invalid sequences are replaced with the Unicode replacement character (U+FFFD).
 *
 * @param str                           UTF-8 string to convert.
 * @return                              Wide string.
 */
std::wstring utf8ToWide(std::string_view str);

/**
 * Convert a string from the given encoding to UTF-8.
 *
 * Invalid or incomplete byte sequences are replaced the Unicode replacement character (U+FFFD).
 *
 * @param str                           String in the source encoding.
 * @param encoding                      Source encoding.
 * @return                              UTF-8 encoded string.
 */
std::string encodedToUtf8(std::string_view str, TextEncoding encoding);

/**
 * Convert a UTF-8 string to the given encoding.
 *
 * Characters that cannot be represented in the target encoding are replaced with `?` or with the Unicode replacement
 * character (U+FFFD).
 *
 * @param str                           UTF-8 string to convert.
 * @param encoding                      Target encoding.
 * @return                              String in the target encoding.
 */
std::string utf8ToEncoded(std::string_view str, TextEncoding encoding);

/**
 * Convert a string from the given encoding to UTF-32.
 *
 * Invalid or incomplete byte sequences are replaced with the Unicode replacement character (U+FFFD).
 *
 * @param str                           String in the source encoding.
 * @param encoding                      Source encoding.
 * @return                              UTF-32 string.
 */
std::u32string encodedToUtf32(std::string_view str, TextEncoding encoding);

/**
 * Convert a UTF-32 string to the given encoding.
 *
 * Characters that cannot be represented in the target encoding are replaced with `?` or with the Unicode replacement
 * character (U+FFFD).
 *
 * @param str                           UTF-32 string to convert.
 * @param encoding                      Target encoding.
 * @return                              String in the target encoding.
 */
std::string utf32ToEncoded(std::u32string_view str, TextEncoding encoding);

/**
 * Decode a single byte in the given encoding into a Unicode code point.
 *
 * Uses the same replacement semantics as `encodedToUtf32`.
 *
 * @param c                             Byte to decode.
 * @param encoding                      Source encoding.
 * @return                              Decoded code point, or the replacement character (U+FFFD) if the byte doesn't
 *                                      decode into exactly one code point, e.g. if it's not mapped in the source
 *                                      encoding, or is a part of a multi-byte sequence.
 */
char32_t encodedToChar32(char c, TextEncoding encoding);

/**
 * Decodes the UTF-8 code point at `*pos` in `s`, and advances `*pos` past it.
 *
 * Iterating a string rune by rune yields exactly the code points that `encodedToUtf32(s, ENCODING_UTF8)` returns,
 * malformed input included.
 *
 * Hand-rolled rather than implemented via ztd.text: doing it there would either pull `<ztd/text.hpp>` into this
 * widely-included header, polluting every consumer with the ztd namespace and its compile-time cost, or move the
 * function out of line - and this is a per-character call in text rendering loops, where the inline version measures
 * 3-6x faster than an out-of-line ztd.text implementation (~1ns vs ~4-5ns per code point, gcc 15 -O3).
 *
 * @param s                             UTF-8 string.
 * @param[in,out] pos                   Byte offset to decode at, must be less than `s.size()`. Advanced past the
 *                                      decoded code point - always by at least one byte, so a loop that stops at
 *                                      `s.size()` is guaranteed to terminate.
 * @return                              Decoded code point, or the replacement character (U+FFFD) for malformed input.
 */
inline char32_t nextRune(std::string_view s, size_t *pos) {
    assert(pos && *pos < s.size());

    // Reject overlong encodings, UTF-16 surrogates, and out-of-range code points.
    static constexpr char32_t minValue[] = {0, 0, 0x80, 0x800, 0x10000};

    size_t cur = *pos;
    unsigned char lead = static_cast<unsigned char>(s[cur++]);
    if (lead < 0x80) { // ASCII.
        *pos = cur;
        return lead;
    }

    size_t length;
    char32_t result;
    if ((lead & 0xE0) == 0xC0) {
        length = 2;
        result = lead & 0x1F;
    } else if ((lead & 0xF0) == 0xE0) {
        length = 3;
        result = lead & 0x0F;
    } else if ((lead & 0xF8) == 0xF0) {
        length = 4;
        result = lead & 0x07;
    } else {
        goto failed; // Not a valid lead byte.
    }

    if (cur + length - 1 > s.size())
        goto failed; // Sequence truncated by the end of the string.

    for (size_t end = cur + length - 1; cur != end; cur++) {
        unsigned char continuation = static_cast<unsigned char>(s[cur]);
        if ((continuation & 0xC0) != 0x80)
            goto failed; // Sequence interrupted by a non-continuation byte.
        result = (result << 6) | (continuation & 0x3F);
    }

    if (result < minValue[length] || (result >= 0xD800 && result <= 0xDFFF) || result > 0x10FFFF)
        goto failed; // Overlong encoding, UTF-16 surrogate, or out-of-range code point.

    *pos = cur;
    return result;

failed:
    // Matches `encodedToUtf32`: an ill-formed sequence yields a single replacement character, consuming the
    // offending bytes together with the following run of bytes that cannot start a new sequence - continuation
    // bytes, plus `C0`/`C1`, which never appear in well-formed UTF-8 at any position.
    while (cur < s.size()) {
        unsigned char b = static_cast<unsigned char>(s[cur]);
        if ((b & 0xC0) != 0x80 && b != 0xC0 && b != 0xC1)
            break;
        cur++;
    }
    *pos = cur;
    return 0xFFFD;
}

} // namespace txt
