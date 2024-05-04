#include <string>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Utility/String.h"

UNIT_TEST(String, toPrintable) {
    EXPECT_EQ(toPrintable("123\xFF", '.'), "123.");
}

UNIT_TEST(String, toHexDump) {
    EXPECT_EQ(toHexDump("1234", 2), "3132 3334");
    EXPECT_EQ(toHexDump("0000"), "30303030");
}

UNIT_TEST(String, replaceAll) {
    EXPECT_EQ(replaceAll("123", "1", "123"), "12323");
    EXPECT_EQ(replaceAll("123", "10", "100"), "123");
    EXPECT_EQ(replaceAll("ab123ab", "ab", "zz"), "zz123zz");
    EXPECT_EQ(replaceAll("AAAA", "AA", "AAZAA"), "AAZAAAAZAA");
}

UNIT_TEST(String, split) {
    std::vector<std::string_view> v;

    splitString("aa;bb;cc", ';', &v);
    std::vector<std::string_view> r0 = {"aa", "bb", "cc"};
    EXPECT_EQ(v, r0);

    splitString("ABC", ';', &v);
    std::vector<std::string_view> r1 = {"ABC"};
    EXPECT_EQ(v, r1);

    splitString("AB", 'B', &v);
    std::vector<std::string_view> r2 = {"A", ""};
    EXPECT_EQ(v, r2);

    splitString("", ';', &v);
    EXPECT_TRUE(v.empty());
}

UNIT_TEST(String, join) {
    EXPECT_EQ(join("", '/', std::string_view("123"), std::string("321")), "/123321");
}





