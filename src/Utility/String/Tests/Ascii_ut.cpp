#include <string>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Utility/String/Ascii.h"

UNIT_TEST(Ascii, toLowerUpperString) {
    EXPECT_EQ(ascii::toLower("123"), "123");
    EXPECT_EQ(ascii::toLower("ABCd"), "abcd");
    EXPECT_EQ(ascii::toLower("z"), "z");
    EXPECT_EQ(ascii::toUpper("123"), "123");
    EXPECT_EQ(ascii::toUpper("ABCd"), "ABCD");
    EXPECT_EQ(ascii::toUpper("Z"), "Z");

    // toLower/toUpper should do nothing for non-ascii chars.
    for (int i = 128; i < 255; i++) {
        const char string[2] = { static_cast<char>(i), 0 };
        EXPECT_EQ(ascii::toLower(string), string);
        EXPECT_EQ(ascii::toUpper(string), string);
    }
}

UNIT_TEST(Ascii, noCaseEquals) {
    EXPECT_FALSE(ascii::noCaseEquals("abc", "abcd"));
    EXPECT_FALSE(ascii::noCaseEquals("abd", "abc"));
    EXPECT_TRUE(ascii::noCaseEquals("abc\0\0", "abc"));
    EXPECT_TRUE(ascii::noCaseEquals("ABC", "abc"));
    EXPECT_TRUE(ascii::noCaseEquals("Abc", "abC"));
    EXPECT_TRUE(ascii::noCaseEquals("123ab..?z", "123Ab..?Z"));
    EXPECT_TRUE(ascii::noCaseEquals("", ""));
    EXPECT_FALSE(ascii::noCaseEquals("", "Z"));
    EXPECT_FALSE(ascii::noCaseEquals("@", "`")); // \x40 vs \x60
}

UNIT_TEST(Ascii, noCaseLess) {
    EXPECT_TRUE(ascii::noCaseLess("A", "AB"));
    EXPECT_FALSE(ascii::noCaseLess("AB", "A"));
    EXPECT_TRUE(ascii::noCaseLess("a", "B"));
    EXPECT_FALSE(ascii::noCaseLess("B", "a"));
    EXPECT_FALSE(ascii::noCaseLess("b", "B"));
    EXPECT_FALSE(ascii::noCaseLess("B", "b"));
    EXPECT_TRUE(ascii::noCaseLess("@", "`"));
}

UNIT_TEST(Ascii, toPrintable) {
    EXPECT_EQ(ascii::toPrintable("123\xFF", '.'), "123.");
}



