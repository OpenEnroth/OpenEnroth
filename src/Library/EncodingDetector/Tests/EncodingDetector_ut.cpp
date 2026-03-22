#include <optional>

#include "Testing/Unit/UnitTest.h"

#include "Library/EncodingDetector/EncodingDetector.h"

static TextEncoding detect(std::string_view text) {
    EncodingDetector detector;
    detector.write(text);
    return detector.finish().encoding;
}

UNIT_TEST(EncodingDetector, Cp1251) {
    EXPECT_EQ(detect("\xc0\xe2\xe5\xeb\xfc"), ENCODING_WINDOWS_1251); // "Авель" (Abel in Russian)
    EXPECT_EQ(detect("\xc0\xeb\xe5\xea\xf1\xe0\xed\xe4\xf0"), ENCODING_WINDOWS_1251); // "Александр" (Alexander in Russian)
    EXPECT_EQ(detect("\xc8\xe2\xe0\xed"), ENCODING_WINDOWS_1251); // "Иван" (Ivan in Russian)
    EXPECT_EQ(detect("\xcc\xe0\xf0\xe8\xff"), ENCODING_WINDOWS_1251); // "Мария" (Maria in Russian)
    EXPECT_EQ(detect("\xc1\xee\xf0\xe8\xf1"), ENCODING_WINDOWS_1251); // "Борис" (Boris in Russian)
    EXPECT_EQ(detect("\xc7\xee\xeb\xf2\xe0\xed"), ENCODING_WINDOWS_1251); // "Золтан" (Zoltan in Russian)
    EXPECT_EQ(detect("\xc0\xeb\xe5\xea\xf1\xe8\xf1"), ENCODING_WINDOWS_1251); // "Алексис" (Alexis in Russian)
    EXPECT_EQ(detect("\xcf\xe0\xf0\xee\xeb\xfc: ABC"), ENCODING_WINDOWS_1251); // "Пароль: ABC" (Password: ABC in Russian)
}

UNIT_TEST(EncodingDetector, Ascii) {
    EXPECT_EQ(detect("Zoltan"), ENCODING_ASCII);
    EXPECT_EQ(detect("Alexis"), ENCODING_ASCII);
    EXPECT_EQ(detect("Alexander"), ENCODING_ASCII);
}

UNIT_TEST(EncodingDetector, Cp1252) {
    EXPECT_EQ(detect("Fran\xe7ois"), ENCODING_ISO_8859_1); // "François" (French name with cedilla)
    EXPECT_EQ(detect("Ren\xe9"), ENCODING_ISO_8859_2); // "René" (French name with acute)
}

UNIT_TEST(EncodingDetector, EdgeCases) {
    EXPECT_EQ(detect(""), ENCODING_BYTES);
    EXPECT_EQ(detect("A"), ENCODING_ASCII);
}
