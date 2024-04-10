#include <map>
#include <unordered_map>
#include <type_traits>

#include "Testing/Unit/UnitTest.h"

#include "Utility/TransparentFunctors.h"
#include "Utility/MapAccess.h"

template<class Map>
static void runTransparentTest() {
    Map m1;

    EXPECT_FALSE(m1.contains(std::string_view("1")));
    EXPECT_EQ(valueOr(m1, std::string_view("1"), -1), -1);

    m1["1"] = 1;
    m1[std::string("2")] = 2;
    if constexpr (std::is_same_v<typename Map::key_type, TransparentString>) {
        m1[std::string_view("3")] = 3;
    } else {
        m1["3"] = 3; // Can't index with std::string_view pre-C++26.
    }

    EXPECT_EQ(valueOr(m1, std::string_view("1"), 0), 1);
    EXPECT_EQ(valueOr(m1, std::string("2"), 0), 2);
    EXPECT_EQ(valueOr(m1, "3", 0), 3);
}

UNIT_TEST(TransparentFunctors, UnorderedMap) {
    runTransparentTest<std::unordered_map<std::string, int, TransparentStringHash, TransparentStringEquals>>();
}

UNIT_TEST(TransparentFunctors, UnorderedMapWithTransparentString) {
    runTransparentTest<std::unordered_map<TransparentString, int, TransparentStringHash, TransparentStringEquals>>();
}

UNIT_TEST(TransparentFunctors, Map) {
    runTransparentTest<std::map<std::string, int, TransparentStringLess>>();
}

UNIT_TEST(TransparentFunctors, MapWithTransparentString) {
    runTransparentTest<std::map<TransparentString, int, TransparentStringLess>>();
}
