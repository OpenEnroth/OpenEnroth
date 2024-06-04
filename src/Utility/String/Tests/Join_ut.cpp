#include <string>
#include <vector>
#include <set>

#include "Testing/Unit/UnitTest.h"

#include "Utility/String/Join.h"

UNIT_TEST(StringJoin, joinVararg) {
    EXPECT_EQ(join("", '/', std::string_view("123"), std::string("321")), "/123321");
}

UNIT_TEST(StringJoin, joinContainer) {
    std::vector<std::string> v1 = {"1", "2"};
    EXPECT_EQ(join(v1, '/'), "1/2");

    std::vector<std::string> v2;
    EXPECT_EQ(join(v2, '/'), "");

    std::vector<std::string_view> v3 = {"1", "2"};
    EXPECT_EQ(join(v3, '/'), "1/2");

    std::set<std::string_view> v4 = {"4", "1", "99"};
    EXPECT_EQ(join(v4, '/'), "1/4/99");
}


