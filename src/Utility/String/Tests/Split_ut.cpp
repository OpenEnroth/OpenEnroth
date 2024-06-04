#include <string>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Utility/String/Split.h"

UNIT_TEST(StringSplit, split) {
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
