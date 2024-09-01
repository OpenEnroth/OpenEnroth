#include <numeric>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Segment.h"

UNIT_TEST(Segment, Iteration) {
    enum class Index { AA, BB, CC };
    using enum Index;

    int counter = 0;
    for(Index i : Segment(AA, CC))
        counter++;
    EXPECT_EQ(counter, 3);

    counter = 0;
    for(Index i : Segment(AA, AA))
        counter++;
    EXPECT_EQ(counter, 1);

    std::vector<int> v;
    for(int i : Segment(0, 99))
        v.push_back(i);
    std::vector<int> w(100, 0);
    std::iota(w.begin(), w.end(), 0);
    EXPECT_EQ(v, w);
}

UNIT_TEST(Segment, Contains) {
    auto seg = Segment(0, 2);

    EXPECT_MISSES(seg, -1);
    EXPECT_CONTAINS(seg, 0);
    EXPECT_CONTAINS(seg, 1);
    EXPECT_CONTAINS(seg, 2);
    EXPECT_MISSES(seg, 3);
}

UNIT_TEST(Segment, Constexpr) {
    constexpr auto seg = Segment(0, 2);
    constexpr bool b = seg.contains(1);

    EXPECT_TRUE(b);
}

UNIT_TEST(Segment, Empty) {
    constexpr Segment emptySegment = Segment<int>();

    EXPECT_MISSES(emptySegment, 0);
    EXPECT_MISSES(emptySegment, 1);
    for (int i : emptySegment) {
        EXPECT_FALSE(true);
    }
    EXPECT_EQ(emptySegment.size(), 0);
}

UNIT_TEST(Segment, Size) {
    constexpr Segment segment(1, 3);
    EXPECT_EQ(segment.size(), 3);
}
