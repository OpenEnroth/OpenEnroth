#include <gtest/gtest.h>
#include <memory>
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
