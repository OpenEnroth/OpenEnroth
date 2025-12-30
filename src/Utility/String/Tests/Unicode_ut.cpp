#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Utility/String/Unicode.h"

UNIT_TEST(Unicode, InvalidUtf8) {
    EXPECT_EQ(unicode::utf8toWide("\xc3\x28"), L"\xfffd(");
}

UNIT_TEST(Unicode, InvalidUtf16) {
    // On Windows (16-bit wchar_t), \xDC00 is an unpaired surrogate.
    // On Unix (32-bit wchar_t), \xDC00 is a valid (but reserved) code point.
    // The encoding should still work, producing a replacement character for the surrogate.
    EXPECT_EQ(unicode::wideToUtf8(L"\xDC00\x0028"), "\xef\xbf\xbd(");  // \xef\xbf\xbd is U+FFFD.
}

UNIT_TEST(Unicode, Empty) {
    EXPECT_EQ(unicode::utf8toWide(""), L"");
    EXPECT_EQ(unicode::wideToUtf8(L""), "");
}

UNIT_TEST(Unicode, Substring) {
    // Test that transcoding a substring (part of a longer string) works correctly.
    // This catches issues where the transcoding might read past the substring boundaries.
    std::string fullString = "Hello, World!";
    std::string_view substring = std::string_view(fullString).substr(0, 5);  // "Hello"
    EXPECT_EQ(unicode::utf8toWide(substring), L"Hello");

    std::wstring fullWideString = L"Hello, World!";
    std::wstring_view wideSubstring = std::wstring_view(fullWideString).substr(0, 5);  // L"Hello"
    EXPECT_EQ(unicode::wideToUtf8(wideSubstring), "Hello");
}
