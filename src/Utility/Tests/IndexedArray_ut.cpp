#include <gtest/gtest.h>
#include <stddef.h>
#include <vector>
#include "Testing/Unit/UnitTest.h"
#include "Utility/IndexedArray.h"
#include "Utility/Segment.h"

UNIT_TEST(IndexedArray, Size) {
    EXPECT_EQ(sizeof(IndexedArray<int, 1, 1>), sizeof(int));
    EXPECT_EQ(sizeof(IndexedArray<int, 0, 10>), 11 * sizeof(int));
    EXPECT_EQ(sizeof(IndexedArray<int, 1, 7>), 7 * sizeof(int));
    EXPECT_EQ(sizeof(IndexedArray<int, -10, 10>), 21 * sizeof(int));

    enum class Index { A = 1000, B, C, D };
    using enum Index;

    EXPECT_EQ(sizeof(IndexedArray<int, A, D>), 4 * sizeof(int));
}

UNIT_TEST(IndexedArray, NonZeroIndex) {
    enum class Index {
        AA = -10,
        XX = 0,
        BB = 10
    };
    using enum Index;

    IndexedArray<int, AA, BB> a = {{}};

    EXPECT_EQ(a[AA], 0);
    EXPECT_EQ(a[BB], 0);
    EXPECT_EQ(a[XX], 0);

    for(int &val : a) {
        val = -1;
        break;
    }

    EXPECT_EQ(a[AA], -1);
}

UNIT_TEST(IndexedArray, MapInit) {
    enum class Index { A, B, C, D };
    using enum Index;

    IndexedArray<int, A, D> a = {
        {A, 0},
        {D, 1},
        {C, 2},
        {B, 3}
    };

    EXPECT_EQ(a[A], 0);
    EXPECT_EQ(a[B], 3);
    EXPECT_EQ(a[C], 2);
    EXPECT_EQ(a[D], 1);
}

UNIT_TEST(IndexedArray, Indices) {
    enum class Index { A = 100, B, C, D };
    using enum Index;

    IndexedArray<int, B, D> a;
    std::vector<Index> indices = {B, C, D};
    size_t pos = 0;
    for(Index i : a.indices())
        EXPECT_EQ(i, indices[pos++]);
    EXPECT_EQ(pos, 3);
}
