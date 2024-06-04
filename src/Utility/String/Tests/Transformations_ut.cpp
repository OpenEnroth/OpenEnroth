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




