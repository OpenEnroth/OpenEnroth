#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Library/Serialization/Serialization.h"

#include "Utility/String/Encoding.h"
#include "Utility/Segment.h"

UNIT_TEST(Encoding, WideInvalidUtf8) {
    EXPECT_EQ(txt::utf8ToWide("\xc3\x28"), L"\xfffd(");
}

UNIT_TEST(Encoding, WideInvalidUtf16) {
    // On Windows (16-bit wchar_t), \xDC00 is an unpaired surrogate.
    // On Unix (32-bit wchar_t), \xDC00 is a valid (but reserved) code point.
    // The encoding should still work, producing a replacement character for the surrogate.
    EXPECT_EQ(txt::wideToUtf8(L"\xDC00\x0028"), "\xef\xbf\xbd(");  // \xef\xbf\xbd is U+FFFD.
}

UNIT_TEST(Encoding, WideEmpty) {
    EXPECT_EQ(txt::utf8ToWide(""), L"");
    EXPECT_EQ(txt::wideToUtf8(L""), "");
}

UNIT_TEST(Encoding, WideSubstring) {
    // Test that transcoding a substring (part of a longer string) works correctly.
    // This catches issues where the transcoding might read past the substring boundaries.
    std::string fullString = "Hello, World!";
    std::string_view substring = std::string_view(fullString).substr(0, 5);  // "Hello"
    EXPECT_EQ(txt::utf8ToWide(substring), L"Hello");

    std::wstring fullWideString = L"Hello, World!";
    std::wstring_view wideSubstring = std::wstring_view(fullWideString).substr(0, 5);  // L"Hello"
    EXPECT_EQ(txt::wideToUtf8(wideSubstring), "Hello");
}

UNIT_TEST(Encoding, Ascii) {
    std::string ascii = "Hello, World!";
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_ASCII), ascii);
    EXPECT_EQ(txt::utf8ToEncoded(ascii, ENCODING_ASCII), ascii);

    // Non-ASCII characters should be replaced with '?'
    EXPECT_EQ(txt::utf8ToEncoded("\xc3\xa4", ENCODING_ASCII), "?");  // ä -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_ASCII), "?");  // 中 -> ?

    // Invalid ASCII bytes (>= 0x80) should produce replacement character
    EXPECT_EQ(txt::encodedToUtf8("\x80", ENCODING_ASCII), "\xef\xbf\xbd");  // 0x80 -> U+FFFD
}

UNIT_TEST(Encoding, Utf8Roundtrip) {
    std::string utf8 = "Hello, \xc3\xa4\xc3\xb6\xc3\xbc!";  // "Hello, äöü!"
    EXPECT_EQ(txt::encodedToUtf8(utf8, ENCODING_UTF8), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_UTF8), utf8);

    // Invalid UTF-8 sequences should produce replacement character
    EXPECT_EQ(txt::encodedToUtf8("\xc3\x28", ENCODING_UTF8), "\xef\xbf\xbd(");  // Invalid continuation -> U+FFFD
}

UNIT_TEST(Encoding, Utf16BE) {
    // "Hi" in UTF-16 BE: 0x00 0x48 0x00 0x69
    std::string utf16be = std::string("\x00\x48\x00\x69", 4);
    EXPECT_EQ(txt::encodedToUtf8(utf16be, ENCODING_UTF16_BE), "Hi");

    std::string utf8 = "Hi";
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_UTF16_BE), utf16be);

    // Unpaired high surrogate (0xD800) should produce replacement character
    std::string unpaired = std::string("\xd8\x00\x00\x41", 4);  // High surrogate + 'A'
    EXPECT_EQ(txt::encodedToUtf8(unpaired, ENCODING_UTF16_BE), "\xef\xbf\xbd" "A");

    // Truncated UTF-8 -> UTF-16 BE should produce replacement character (U+FFFD = 0xFF 0xFD in BE)
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8", ENCODING_UTF16_BE), std::string("\xff\xfd", 2));
}

UNIT_TEST(Encoding, Utf16LE) {
    // "Hi" in UTF-16 LE: 0x48 0x00 0x69 0x00
    std::string utf16le = std::string("\x48\x00\x69\x00", 4);
    EXPECT_EQ(txt::encodedToUtf8(utf16le, ENCODING_UTF16_LE), "Hi");

    std::string utf8 = "Hi";
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_UTF16_LE), utf16le);

    // Unpaired high surrogate (0xD800) should produce replacement character
    std::string unpaired = std::string("\x00\xd8\x41\x00", 4);  // High surrogate + 'A'
    EXPECT_EQ(txt::encodedToUtf8(unpaired, ENCODING_UTF16_LE), "\xef\xbf\xbd" "A");

    // Truncated UTF-8 -> UTF-16 LE should produce replacement character
    // U+FFFD in UTF-16 LE is 0xFD 0xFF
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8", ENCODING_UTF16_LE), std::string("\xfd\xff", 2));
}

UNIT_TEST(Encoding, Utf32BE) {
    // "A" in UTF-32 BE: 0x00 0x00 0x00 0x41
    std::string utf32be = std::string("\x00\x00\x00\x41", 4);
    EXPECT_EQ(txt::encodedToUtf8(utf32be, ENCODING_UTF32_BE), "A");

    std::string utf8 = "A";
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_UTF32_BE), utf32be);

    // Surrogate code point (0xD800) followed by valid 'A' (0x41)
    std::string surrogateAndA = std::string("\x00\x00\xd8\x00\x00\x00\x00\x41", 8);
    EXPECT_EQ(txt::encodedToUtf8(surrogateAndA, ENCODING_UTF32_BE), "\xef\xbf\xbd" "A");  // U+FFFD + A

    // Truncated UTF-8 -> UTF-32 BE should produce replacement character
    // U+FFFD in UTF-32 BE is 0x00 0x00 0xFF 0xFD
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8", ENCODING_UTF32_BE), std::string("\x00\x00\xff\xfd", 4));
}

UNIT_TEST(Encoding, Utf32LE) {
    // "A" in UTF-32 LE: 0x41 0x00 0x00 0x00
    std::string utf32le = std::string("\x41\x00\x00\x00", 4);
    EXPECT_EQ(txt::encodedToUtf8(utf32le, ENCODING_UTF32_LE), "A");

    std::string utf8 = "A";
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_UTF32_LE), utf32le);

    // Surrogate code point (0xD800) followed by valid 'A' (0x41)
    std::string surrogateAndA = std::string("\x00\xd8\x00\x00\x41\x00\x00\x00", 8);
    EXPECT_EQ(txt::encodedToUtf8(surrogateAndA, ENCODING_UTF32_LE), "\xef\xbf\xbd" "A");  // U+FFFD + A

    // Truncated UTF-8 -> UTF-32 LE should produce replacement character
    // U+FFFD in UTF-32 LE is 0xFD 0xFF 0x00 0x00
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8", ENCODING_UTF32_LE), std::string("\xfd\xff\x00\x00", 4));
}

UNIT_TEST(Encoding, Iso8859_1) {
    // ISO-8859-1 (Latin-1): 0xE4 = 'ä', 0xF6 = 'ö', 0xFC = 'ü'
    std::string iso = "\xe4\xf6\xfc";
    std::string utf8 = "\xc3\xa4\xc3\xb6\xc3\xbc";  // äöü in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(iso, ENCODING_ISO_8859_1), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_ISO_8859_1), iso);

    // Characters outside ISO-8859-1 (U+0100+) should be replaced with '?'
    EXPECT_EQ(txt::utf8ToEncoded("\xc4\x85", ENCODING_ISO_8859_1), "?");  // ą (U+0105) -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_ISO_8859_1), "?");  // 中 -> ?
}

UNIT_TEST(Encoding, Iso8859_2) {
    // ISO-8859-2 (Latin-2): 0xB1 = 'ą' (Polish a-ogonek)
    std::string iso = "\xb1";
    std::string utf8 = "\xc4\x85";  // ą in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(iso, ENCODING_ISO_8859_2), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_ISO_8859_2), iso);

    // Cyrillic and CJK characters are not in ISO-8859-2
    EXPECT_EQ(txt::utf8ToEncoded("\xd0\xb0", ENCODING_ISO_8859_2), "?");  // а (Cyrillic) -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_ISO_8859_2), "?");  // 中 -> ?
}

UNIT_TEST(Encoding, Iso8859_3) {
    // ISO-8859-3 (Latin-3, South European): 0xB6 = 'ĥ' (h with circumflex)
    std::string iso = "\xb6";
    std::string utf8 = "\xc4\xa5";  // ĥ (U+0125) in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(iso, ENCODING_ISO_8859_3), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_ISO_8859_3), iso);

    // Cyrillic and CJK characters are not in ISO-8859-3
    EXPECT_EQ(txt::utf8ToEncoded("\xd0\xb0", ENCODING_ISO_8859_3), "?");  // а -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_ISO_8859_3), "?");  // 中 -> ?
}

UNIT_TEST(Encoding, Iso8859_4) {
    // ISO-8859-4 (Latin-4, North European): 0xB3 = 'ŗ' (r with cedilla)
    std::string iso = "\xb3";
    std::string utf8 = "\xc5\x97";  // ŗ (U+0157) in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(iso, ENCODING_ISO_8859_4), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_ISO_8859_4), iso);

    // Cyrillic and CJK characters are not in ISO-8859-4
    EXPECT_EQ(txt::utf8ToEncoded("\xd0\xb0", ENCODING_ISO_8859_4), "?");  // а -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_ISO_8859_4), "?");  // 中 -> ?
}

UNIT_TEST(Encoding, Iso8859_5) {
    // ISO-8859-5 (Cyrillic): 0xD0 = 'а' (Cyrillic small a)
    std::string iso = "\xd0";
    std::string utf8 = "\xd0\xb0";  // а in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(iso, ENCODING_ISO_8859_5), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_ISO_8859_5), iso);

    // Latin extended and CJK characters are not in ISO-8859-5
    EXPECT_EQ(txt::utf8ToEncoded("\xc4\x85", ENCODING_ISO_8859_5), "?");  // ą (Polish) -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_ISO_8859_5), "?");  // 中 -> ?
}

UNIT_TEST(Encoding, Iso8859_6) {
    // ISO-8859-6 (Arabic): 0xC1 = 'ء' (Arabic letter hamza)
    std::string iso = "\xc1";
    std::string utf8 = "\xd8\xa1";  // ء (U+0621) in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(iso, ENCODING_ISO_8859_6), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_ISO_8859_6), iso);

    // Cyrillic and CJK characters are not in ISO-8859-6
    EXPECT_EQ(txt::utf8ToEncoded("\xd0\xb0", ENCODING_ISO_8859_6), "?");  // а -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_ISO_8859_6), "?");  // 中 -> ?
}

UNIT_TEST(Encoding, Iso8859_7) {
    // ISO-8859-7 (Greek): 0xE1 = 'α' (Greek small alpha)
    std::string iso = "\xe1";
    std::string utf8 = "\xce\xb1";  // α in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(iso, ENCODING_ISO_8859_7), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_ISO_8859_7), iso);

    // Cyrillic and CJK characters are not in ISO-8859-7
    EXPECT_EQ(txt::utf8ToEncoded("\xd0\xb0", ENCODING_ISO_8859_7), "?");  // а (Cyrillic) -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_ISO_8859_7), "?");  // 中 -> ?
}

UNIT_TEST(Encoding, Iso8859_8) {
    // ISO-8859-8 (Hebrew): 0xE0 = 'א' (Hebrew letter aleph)
    std::string iso = "\xe0";
    std::string utf8 = "\xd7\x90";  // א (U+05D0) in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(iso, ENCODING_ISO_8859_8), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_ISO_8859_8), iso);

    // Cyrillic and CJK characters are not in ISO-8859-8
    EXPECT_EQ(txt::utf8ToEncoded("\xd0\xb0", ENCODING_ISO_8859_8), "?");  // а -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_ISO_8859_8), "?");  // 中 -> ?
}

UNIT_TEST(Encoding, Iso8859_10) {
    // ISO-8859-10 (Latin-6, Nordic): 0xBF = 'ŋ' (eng)
    std::string iso = "\xbf";
    std::string utf8 = "\xc5\x8b";  // ŋ (U+014B) in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(iso, ENCODING_ISO_8859_10), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_ISO_8859_10), iso);

    // Cyrillic and CJK characters are not in ISO-8859-10
    EXPECT_EQ(txt::utf8ToEncoded("\xd0\xb0", ENCODING_ISO_8859_10), "?");  // а -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_ISO_8859_10), "?");  // 中 -> ?
}

UNIT_TEST(Encoding, Iso8859_13) {
    // ISO-8859-13 (Latin-7, Baltic): 0xF0 = 'š' (s with caron)
    std::string iso = "\xf0";
    std::string utf8 = "\xc5\xa1";  // š (U+0161) in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(iso, ENCODING_ISO_8859_13), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_ISO_8859_13), iso);

    // Cyrillic and CJK characters are not in ISO-8859-13
    EXPECT_EQ(txt::utf8ToEncoded("\xd0\xb0", ENCODING_ISO_8859_13), "?");  // а -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_ISO_8859_13), "?");  // 中 -> ?
}

UNIT_TEST(Encoding, Iso8859_15) {
    // ISO-8859-15 (Latin-9): 0xA4 = '€' (Euro sign)
    std::string iso = "\xa4";
    std::string utf8 = "\xe2\x82\xac";  // € (U+20AC) in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(iso, ENCODING_ISO_8859_15), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_ISO_8859_15), iso);

    // Cyrillic and CJK characters are not in ISO-8859-15
    EXPECT_EQ(txt::utf8ToEncoded("\xd0\xb0", ENCODING_ISO_8859_15), "?");  // а -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_ISO_8859_15), "?");  // 中 -> ?
}

UNIT_TEST(Encoding, Iso8859_16) {
    // ISO-8859-16 (Latin-10, South-Eastern European): 0xA4 = '€' (Euro sign)
    std::string iso = "\xa4";
    std::string utf8 = "\xe2\x82\xac";  // € (U+20AC) in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(iso, ENCODING_ISO_8859_16), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_ISO_8859_16), iso);

    // Cyrillic and CJK characters are not in ISO-8859-16
    EXPECT_EQ(txt::utf8ToEncoded("\xd0\xb0", ENCODING_ISO_8859_16), "?");  // а -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_ISO_8859_16), "?");  // 中 -> ?
}

UNIT_TEST(Encoding, Windows1251) {
    // Windows-1251 (Cyrillic): 0xE0 = 'а', 0xE1 = 'б', 0xE2 = 'в'
    std::string win = "\xe0\xe1\xe2";
    std::string utf8 = "\xd0\xb0\xd0\xb1\xd0\xb2";  // абв in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(win, ENCODING_WINDOWS_1251), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_WINDOWS_1251), win);

    // Greek and CJK characters are not in Windows-1251
    EXPECT_EQ(txt::utf8ToEncoded("\xce\xb1", ENCODING_WINDOWS_1251), "?");  // α (Greek) -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_WINDOWS_1251), "?");  // 中 -> ?

    // Undefined byte 0x98 maps to U+FFFF in ztd::text (noncharacter)
    EXPECT_EQ(txt::encodedToUtf8("\x98", ENCODING_WINDOWS_1251), "\xef\xbf\xbf");  // U+FFFF
}

UNIT_TEST(Encoding, Windows1252) {
    // Windows-1252 (Western European): 0x80 = '€' (Euro sign)
    std::string win = "\x80";
    std::string utf8 = "\xe2\x82\xac";  // € in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(win, ENCODING_WINDOWS_1252), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_WINDOWS_1252), win);

    // Cyrillic and CJK characters are not in Windows-1252
    EXPECT_EQ(txt::utf8ToEncoded("\xd0\xb0", ENCODING_WINDOWS_1252), "?");  // а (Cyrillic) -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_WINDOWS_1252), "?");  // 中 -> ?
}

UNIT_TEST(Encoding, Windows1253) {
    // Windows-1253 (Greek): 0xE1 = 'α' (Greek small alpha)
    std::string win = "\xe1";
    std::string utf8 = "\xce\xb1";  // α in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(win, ENCODING_WINDOWS_1253), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_WINDOWS_1253), win);

    // Cyrillic and CJK characters are not in Windows-1253
    EXPECT_EQ(txt::utf8ToEncoded("\xd0\xb0", ENCODING_WINDOWS_1253), "?");  // а (Cyrillic) -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_WINDOWS_1253), "?");  // 中 -> ?

    // Undefined byte 0xAA maps to U+FFFF in ztd::text (noncharacter)
    EXPECT_EQ(txt::encodedToUtf8("\xaa", ENCODING_WINDOWS_1253), "\xef\xbf\xbf");  // U+FFFF
}

UNIT_TEST(Encoding, Windows1255) {
    // Windows-1255 (Hebrew): 0xE0 = 'א' (Hebrew letter aleph)
    std::string win = "\xe0";
    std::string utf8 = "\xd7\x90";  // א (U+05D0) in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(win, ENCODING_WINDOWS_1255), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_WINDOWS_1255), win);

    // Cyrillic and CJK characters are not in Windows-1255
    EXPECT_EQ(txt::utf8ToEncoded("\xd0\xb0", ENCODING_WINDOWS_1255), "?");  // а -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_WINDOWS_1255), "?");  // 中 -> ?
}

UNIT_TEST(Encoding, Windows1256) {
    // Windows-1256 (Arabic): 0xC1 = 'ء' (Arabic letter hamza)
    std::string win = "\xc1";
    std::string utf8 = "\xd8\xa1";  // ء (U+0621) in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(win, ENCODING_WINDOWS_1256), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_WINDOWS_1256), win);

    // Cyrillic and CJK characters are not in Windows-1256
    EXPECT_EQ(txt::utf8ToEncoded("\xd0\xb0", ENCODING_WINDOWS_1256), "?");  // а -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_WINDOWS_1256), "?");  // 中 -> ?
}

UNIT_TEST(Encoding, Windows1257) {
    // Windows-1257 (Baltic): 0xE0 = 'ą' (a with ogonek)
    std::string win = "\xe0";
    std::string utf8 = "\xc4\x85";  // ą (U+0105) in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(win, ENCODING_WINDOWS_1257), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_WINDOWS_1257), win);

    // Cyrillic and CJK characters are not in Windows-1257
    EXPECT_EQ(txt::utf8ToEncoded("\xd0\xb0", ENCODING_WINDOWS_1257), "?");  // а -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_WINDOWS_1257), "?");  // 中 -> ?
}

UNIT_TEST(Encoding, Windows1258) {
    // Windows-1258 (Vietnamese): 0xC0 = 'À' (A with grave)
    std::string win = "\xc0";
    std::string utf8 = "\xc3\x80";  // À (U+00C0) in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(win, ENCODING_WINDOWS_1258), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_WINDOWS_1258), win);

    // Cyrillic and CJK characters are not in Windows-1258
    EXPECT_EQ(txt::utf8ToEncoded("\xd0\xb0", ENCODING_WINDOWS_1258), "?");  // а -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_WINDOWS_1258), "?");  // 中 -> ?
}

UNIT_TEST(Encoding, Ibm865) {
    // IBM865 (DOS Nordic) - test multiple high-byte characters for roundtrip:
    // 0x80=Ç 0x81=ü 0x82=é 0x83=â 0x84=ä 0x85=à 0x86=å 0x87=ç 0x91=æ 0x92=Æ
    std::string ibm = "\x80\x81\x82\x83\x84\x85\x86\x87\x91\x92";
    std::string utf8 = "\xc3\x87\xc3\xbc\xc3\xa9\xc3\xa2\xc3\xa4\xc3\xa0\xc3\xa5\xc3\xa7\xc3\xa6\xc3\x86"; // ÇüéâäàåçæÆ in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(ibm, ENCODING_IBM865), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_IBM865), ibm);

    // Cyrillic and CJK characters are not in IBM865
    EXPECT_EQ(txt::utf8ToEncoded("\xd0\xb0", ENCODING_IBM865), "?");  // а (Cyrillic) -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_IBM865), "?");  // 中 -> ?
}

UNIT_TEST(Encoding, Ibm866) {
    // IBM866 (DOS Cyrillic): 0xA0 = 'а', 0xA1 = 'б', 0xA2 = 'в'
    std::string ibm = "\xa0\xa1\xa2";
    std::string utf8 = "\xd0\xb0\xd0\xb1\xd0\xb2";  // абв in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(ibm, ENCODING_IBM866), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_IBM866), ibm);

    // Greek and CJK characters are not in IBM866
    EXPECT_EQ(txt::utf8ToEncoded("\xce\xb1", ENCODING_IBM866), "?");  // α (Greek) -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_IBM866), "?");  // 中 -> ?
}

UNIT_TEST(Encoding, Big5) {
    // Big5: 0xA4 0xA4 = '中' (Chinese character for "middle")
    std::string big5 = "\xa4\xa4";
    std::string utf8 = "\xe4\xb8\xad";  // 中 in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(big5, ENCODING_BIG5), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_BIG5), big5);

    // Arabic characters are not in Big5
    EXPECT_EQ(txt::utf8ToEncoded("\xd8\xa7", ENCODING_BIG5), "?");  // ا (Arabic alef) -> ?

    // Truncated multi-byte sequence should produce replacement character
    EXPECT_EQ(txt::encodedToUtf8("\xa4", ENCODING_BIG5), "\xef\xbf\xbd");  // Truncated -> U+FFFD
}

UNIT_TEST(Encoding, EucKr) {
    // EUC-KR: 0xB0 0xA1 = '가' (Korean syllable GA)
    std::string euckr = "\xb0\xa1";
    std::string utf8 = "\xea\xb0\x80";  // 가 in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(euckr, ENCODING_EUC_KR), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_EUC_KR), euckr);

    // Arabic characters are not in EUC-KR
    EXPECT_EQ(txt::utf8ToEncoded("\xd8\xa7", ENCODING_EUC_KR), "?");  // ا (Arabic alef) -> ?

    // Truncated multi-byte sequence should produce replacement character
    EXPECT_EQ(txt::encodedToUtf8("\xb0", ENCODING_EUC_KR), "\xef\xbf\xbd");  // Truncated -> U+FFFD
}

UNIT_TEST(Encoding, Gb18030) {
    // GB18030: 0xD6 0xD0 = '中' (Chinese character for "middle")
    std::string gb = "\xd6\xd0";
    std::string utf8 = "\xe4\xb8\xad";  // 中 in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(gb, ENCODING_GB18030), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_GB18030), gb);

    // GB18030 can encode all Unicode characters (unlike other CJK encodings)
    // Cyrillic: а (U+0430) -> 0xA7 0xD1 in GB18030
    EXPECT_EQ(txt::utf8ToEncoded("\xd0\xb0", ENCODING_GB18030), "\xa7\xd1");
    EXPECT_EQ(txt::encodedToUtf8("\xa7\xd1", ENCODING_GB18030), "\xd0\xb0");

    // Arabic: ا (U+0627) -> 4-byte sequence in GB18030
    std::string arabicGb = txt::utf8ToEncoded("\xd8\xa7", ENCODING_GB18030);
    EXPECT_EQ(txt::encodedToUtf8(arabicGb, ENCODING_GB18030), "\xd8\xa7");  // Roundtrip

    // Truncated GB18030 -> UTF-8 should produce replacement character
    EXPECT_EQ(txt::encodedToUtf8("\xd6", ENCODING_GB18030), "\xef\xbf\xbd");  // Truncated 2-byte -> U+FFFD
    EXPECT_EQ(txt::encodedToUtf8("\x81\x30", ENCODING_GB18030), "\xef\xbf\xbd");  // Truncated 4-byte (after byte 2) -> U+FFFD
    EXPECT_EQ(txt::encodedToUtf8("\x81\x30\x81", ENCODING_GB18030), "\xef\xbf\xbd");  // Truncated 4-byte (after byte 3) -> U+FFFD

    // Truncated UTF-8 -> GB18030 should produce replacement character
    // GB18030 encodes U+FFFD as 0x84 0x31 0xA4 0x37
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8", ENCODING_GB18030), "\x84\x31\xa4\x37");  // Truncated UTF-8 -> U+FFFD in GB18030
}

UNIT_TEST(Encoding, ShiftJis) {
    // Shift_JIS: 0x82 0xA0 = 'あ' (Hiragana A)
    std::string sjis = "\x82\xa0";
    std::string utf8 = "\xe3\x81\x82";  // あ in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(sjis, ENCODING_SHIFT_JIS), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_SHIFT_JIS), sjis);

    // Arabic characters are not in Shift_JIS
    EXPECT_EQ(txt::utf8ToEncoded("\xd8\xa7", ENCODING_SHIFT_JIS), "?");  // ا (Arabic alef) -> ?

    // Truncated multi-byte sequence should produce replacement character
    EXPECT_EQ(txt::encodedToUtf8("\x82", ENCODING_SHIFT_JIS), "\xef\xbf\xbd");  // Truncated -> U+FFFD
}

UNIT_TEST(Encoding, Koi8r) {
    // KOI8-R (Russian): 0xC1 = 'а', 0xC2 = 'б', 0xD7 = 'в'
    std::string koi = "\xc1\xc2\xd7";
    std::string utf8 = "\xd0\xb0\xd0\xb1\xd0\xb2";  // абв in UTF-8
    EXPECT_EQ(txt::encodedToUtf8(koi, ENCODING_KOI8_R), utf8);
    EXPECT_EQ(txt::utf8ToEncoded(utf8, ENCODING_KOI8_R), koi);

    // Greek and CJK characters are not in KOI8-R
    EXPECT_EQ(txt::utf8ToEncoded("\xce\xb1", ENCODING_KOI8_R), "?");  // α (Greek) -> ?
    EXPECT_EQ(txt::utf8ToEncoded("\xe4\xb8\xad", ENCODING_KOI8_R), "?");  // 中 -> ?
}

UNIT_TEST(Encoding, EmptyEncodedStrings) {
    // Test empty strings for all supported encodings.
    for (TextEncoding encoding : Segment(ENCODING_FIRST, ENCODING_LAST)) {
        // TODO(captainurist): it kinda sucks we can't just do toDebugString for enums via magic enum in Utility. Fix this?
        EXPECT_EQ(txt::encodedToUtf8("", encoding), "") << "encoding: " << static_cast<int>(encoding);
        EXPECT_EQ(txt::utf8ToEncoded("", encoding), "") << "encoding: " << static_cast<int>(encoding);
    }
}

UNIT_TEST(Encoding, AsciiSubset) {
    // ASCII subset (0x00-0x7F) should be preserved in all single-byte and CJK encodings.
    // UTF-16/UTF-32 excluded as they use different byte representations.
    std::string ascii = "Hello123";
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_ASCII), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_UTF8), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_ISO_8859_1), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_ISO_8859_2), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_ISO_8859_3), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_ISO_8859_4), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_ISO_8859_5), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_ISO_8859_6), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_ISO_8859_7), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_ISO_8859_8), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_ISO_8859_10), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_ISO_8859_13), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_ISO_8859_15), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_ISO_8859_16), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_WINDOWS_1251), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_WINDOWS_1252), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_WINDOWS_1253), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_WINDOWS_1255), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_WINDOWS_1256), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_WINDOWS_1257), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_WINDOWS_1258), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_IBM865), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_IBM866), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_BIG5), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_EUC_KR), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_GB18030), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_SHIFT_JIS), ascii);
    EXPECT_EQ(txt::encodedToUtf8(ascii, ENCODING_KOI8_R), ascii);
}
