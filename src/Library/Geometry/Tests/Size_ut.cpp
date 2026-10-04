#include "Testing/Unit/UnitTest.h"

#include "Library/Geometry/Size.h"

UNIT_TEST(Size, IsEmpty) {
    EXPECT_TRUE(Sizei().isEmpty());
    EXPECT_TRUE(Sizei(0, 10).isEmpty());
    EXPECT_TRUE(Sizei(10, -1).isEmpty());
    EXPECT_FALSE(Sizei(1, 1).isEmpty());
}

UNIT_TEST(Size, Bool) {
    EXPECT_FALSE(static_cast<bool>(Sizei()));
    EXPECT_FALSE(static_cast<bool>(Sizei(0, 10)));
    EXPECT_FALSE(static_cast<bool>(Sizei(10, -1)));
    EXPECT_TRUE(static_cast<bool>(Sizei(1, 1)));
    EXPECT_TRUE(static_cast<bool>(Sizef(0.5f, 0.5f)));

    EXPECT_TRUE(!Sizei());
    EXPECT_FALSE(!Sizei(1, 1));

    static_assert(!static_cast<bool>(Sizei()));
    static_assert(static_cast<bool>(Sizei(1, 1)));
    static_assert(!Sizei());
}
