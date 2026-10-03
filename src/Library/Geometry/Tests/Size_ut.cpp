#include "Testing/Unit/UnitTest.h"

#include "Library/Geometry/Size.h"

UNIT_TEST(Size, IsEmpty) {
    EXPECT_TRUE(Sizei().isEmpty());
    EXPECT_TRUE(Sizei(0, 10).isEmpty());
    EXPECT_TRUE(Sizei(10, -1).isEmpty());
    EXPECT_FALSE(Sizei(1, 1).isEmpty());
}

UNIT_TEST(Size, Bool) {
    EXPECT_FALSE(Sizei());
    EXPECT_FALSE(Sizei(0, 10));
    EXPECT_FALSE(Sizei(10, -1));
    EXPECT_TRUE(Sizei(1, 1));
    EXPECT_TRUE(!Sizei());
    EXPECT_FALSE(!Sizef(0.5f, 0.5f));

    static_assert(!Sizei());
    static_assert(static_cast<bool>(Sizei(1, 1)));
}
