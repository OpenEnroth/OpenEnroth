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
