#pragma once

#include <filesystem>
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
 * Convert a UTF-16 string to UTF-8.
 *
 * Invalid sequences, unpaired surrogates included, are replaced with the Unicode replacement character (U+FFFD).
 *
 * @param str                           UTF-16 string to convert.
 * @return                              UTF-8 encoded string.
 */
std::string utf16ToUtf8(std::u16string_view str);

/**
 * Convert a UTF-8 string to UTF-16.
 *
 * Invalid sequences are replaced with the Unicode replacement character (U+FFFD).
 *
 * @param str                           UTF-8 string to convert.
 * @return                              UTF-16 string.
 */
std::u16string utf8ToUtf16(std::string_view str);

/**
 * Convert a WTF-16 string to WTF-8. WTF-16 is any `char16_t` sequence, and WTF-8 is UTF-8 extended so that all of
 * them can be encoded, so this round-trips through `wtf8ToWtf16` whatever it's given.
 *
 * Use this for file names on Windows - Win32 does no Unicode validation on them, so a name can hold unpaired
 * surrogates and non-characters alike. `utf16ToUtf8` would replace the former, giving a name that can't be opened.
 *
 * @param str                           WTF-16 string to convert.
 * @return                              WTF-8 encoded string.
 */
std::string wtf16ToWtf8(std::u16string_view str);

/**
 * Convert a WTF-8 string to WTF-16, the exact inverse of `wtf16ToWtf8`. Valid UTF-8 is also valid WTF-8, so this
 * works on UTF-8 input too.
 *
 * @param str                           WTF-8 string to convert.
 * @return                              WTF-16 string.
 */
std::u16string wtf8ToWtf16(std::string_view str);

/**
 * @param str                           WTF-8 string.
 * @return                              `str` as a native path.
 */
std::filesystem::path wtf8ToPath(std::string_view str);

/**
 * The inverse of `wtf8ToPath`. Unlike `std::filesystem::path::generic_string()`, this never throws - on Windows a
 * name with an unpaired surrogate in it is encoded instead of being rejected.
 *
 * @param path                          Native path.
 * @return                              `path` as a WTF-8 string, always using forward slashes.
 */
std::string pathToWtf8(const std::filesystem::path &path);

#ifdef _WINDOWS
static_assert(sizeof(wchar_t) == sizeof(char16_t));

std::string wideToUtf8(std::wstring_view str);
std::string wideToWtf8(std::wstring_view str);
std::wstring utf8ToWide(std::string_view str);
std::wstring wtf8ToWide(std::string_view str);
#endif

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

} // namespace txt
