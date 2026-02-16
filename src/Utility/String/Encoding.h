#pragma once

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

std::string encodedToUtf8(std::string_view str, TextEncoding encoding);

std::string utf8ToEncoded(std::string_view str, TextEncoding encoding);

} // namespace txt
