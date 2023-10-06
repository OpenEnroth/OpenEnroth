#include "Testing/Unit/UnitTest.h"

#include "Utility/String.h"

UNIT_TEST(String, iequals) {
    EXPECT_FALSE(iequals("abc", "abcd"));
    EXPECT_FALSE(iequals("abd", "abc"));
    EXPECT_TRUE(iequals("abc\0\0", "abc"));
    EXPECT_TRUE(iequals("ABC", "abc"));
    EXPECT_TRUE(iequals("Abc", "abC"));
    EXPECT_TRUE(iequals("123ab..?z", "123Ab..?Z"));
    EXPECT_TRUE(iequals("", ""));
    EXPECT_FALSE(iequals("", "Z"));
    EXPECT_FALSE(iequals("@", "`")); // \x40 vs \x60
}

UNIT_TEST(String, iless) {
    EXPECT_TRUE(iless("A", "AB"));
    EXPECT_FALSE(iless("AB", "A"));
    EXPECT_TRUE(iless("a", "B"));
    EXPECT_FALSE(iless("B", "a"));
    EXPECT_FALSE(iless("b", "B"));
    EXPECT_FALSE(iless("B", "b"));
    EXPECT_TRUE(iless("@", "`"));
}

UNIT_TEST(String, Printable) {
    EXPECT_EQ(toPrintable("123\xFF", '.'), "123.");
}

UNIT_TEST(String, HexDump) {
    EXPECT_EQ(toHexDump("1234", 2), "3132 3334");
    EXPECT_EQ(toHexDump("0000"), "30303030");
}

UNIT_TEST(String, ReplaceAll) {
    EXPECT_EQ(replaceAll("123", "1", "123"), "12323");
    EXPECT_EQ(replaceAll("123", "10", "100"), "123");
    EXPECT_EQ(replaceAll("ab123ab", "ab", "zz"), "zz123zz");
    EXPECT_EQ(replaceAll("AAAA", "AA", "AAZAA"), "AAZAAAAZAA");
}
