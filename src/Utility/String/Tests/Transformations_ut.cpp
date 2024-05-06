#include <string>
#include <vector>
#include <set>

#include "Testing/Unit/UnitTest.h"

#include "Utility/String/Transformations.h"

UNIT_TEST(StringTransformations, toHexDump) {
    EXPECT_EQ(toHexDump("1234", 2), "3132 3334");
    EXPECT_EQ(toHexDump("0000"), "30303030");
}

UNIT_TEST(StringTransformations, replaceAll) {
    EXPECT_EQ(replaceAll("123", "1", "123"), "12323");
    EXPECT_EQ(replaceAll("123", "10", "100"), "123");
    EXPECT_EQ(replaceAll("ab123ab", "ab", "zz"), "zz123zz");
    EXPECT_EQ(replaceAll("AAAA", "AA", "AAZAA"), "AAZAAAAZAA");
}

UNIT_TEST(StringTransformations, split) {
    std::vector<std::string_view> v;

    split("aa;bb;cc", ';', &v);
    std::vector<std::string_view> r0 = {"aa", "bb", "cc"};
    EXPECT_EQ(v, r0);

    split("ABC", ';', &v);
    std::vector<std::string_view> r1 = {"ABC"};
    EXPECT_EQ(v, r1);

    split("AB", 'B', &v);
    std::vector<std::string_view> r2 = {"A", ""};
    EXPECT_EQ(v, r2);

    split("", ';', &v);
    std::vector<std::string_view> r3 = {""};
    EXPECT_EQ(v, r3);
}

UNIT_TEST(StringTransformations, join) {
    EXPECT_EQ(join("", '/', std::string_view("123"), std::string("321")), "/123321");
}

UNIT_TEST(StringTransformations, joinSeparator) {
    std::vector<std::string> v1 = {"1", "2"};
    EXPECT_EQ(join(v1, '/'), "1/2");

    std::vector<std::string> v2;
    EXPECT_EQ(join(v2, '/'), "");

    std::vector<std::string_view> v3 = {"1", "2"};
    EXPECT_EQ(join(v3, '/'), "1/2");

    std::set<std::string_view> v4 = {"4", "1", "99"};
    EXPECT_EQ(join(v4, '/'), "1/4/99");
}




