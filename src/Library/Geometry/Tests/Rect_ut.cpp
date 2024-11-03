#include <memory>
#include <utility>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/Geometry/Rect.h"

UNIT_TEST(Rect, Contains) {
    EXPECT_FALSE(Recti().contains(Pointi())); // Empty rect contains nothing.

    EXPECT_TRUE(Recti(0, 0, 1, 1).contains(Pointi(0, 0)));
    EXPECT_FALSE(Recti(0, 0, 1, 1).contains(Pointi(0, 1)));
    EXPECT_FALSE(Recti(0, 0, 1, 1).contains(Pointi(1, 0)));
    EXPECT_FALSE(Recti(0, 0, 1, 1).contains(Pointi(1, 1)));
}

UNIT_TEST(Rect, Intersects) {
    EXPECT_FALSE(Recti().intersects(Recti())); // Empty rect intersects nothing.

    EXPECT_TRUE(Recti(0, 0, 2, 2).intersects(Recti(1, 1, 2, 2)));
    EXPECT_TRUE(Recti(1, 1, 2, 2).intersects(Recti(0, 0, 2, 2)));
    EXPECT_TRUE(Recti(0, 1, 2, 2).intersects(Recti(1, 0, 2, 2)));
    EXPECT_TRUE(Recti(1, 0, 2, 2).intersects(Recti(0, 1, 2, 2)));

    // Touching rects don't intersect.
    EXPECT_FALSE(Recti(0, 0, 1, 1).intersects(Recti(-1, -1, 1, 1)));
    EXPECT_FALSE(Recti(0, 0, 1, 1).intersects(Recti(-1, 0, 1, 1)));
    EXPECT_FALSE(Recti(0, 0, 1, 1).intersects(Recti(-1, 1, 1, 1)));
    EXPECT_FALSE(Recti(0, 0, 1, 1).intersects(Recti(0, -1, 1, 1)));
    EXPECT_FALSE(Recti(0, 0, 1, 1).intersects(Recti(0, 1, 1, 1)));
    EXPECT_FALSE(Recti(0, 0, 1, 1).intersects(Recti(1, -1, 1, 1)));
    EXPECT_FALSE(Recti(0, 0, 1, 1).intersects(Recti(1, 0, 1, 1)));
    EXPECT_FALSE(Recti(0, 0, 1, 1).intersects(Recti(1, 1, 1, 1)));
}

UNIT_TEST(Rect, Intersection) {
    EXPECT_TRUE(Recti(0, 0, 1, 1).intersection(Recti(1, 1, 1, 1)).isEmpty());

    EXPECT_EQ(Recti(0, 0, 2, 2).intersection(Recti(1, 1, 2, 2)), Recti(1, 1, 1, 1));
    EXPECT_EQ(Recti(1, 1, 2, 2).intersection(Recti(0, 0, 2, 2)), Recti(1, 1, 1, 1));
    EXPECT_EQ(Recti(0, 1, 2, 2).intersection(Recti(1, 0, 2, 2)), Recti(1, 1, 1, 1));
    EXPECT_EQ(Recti(1, 0, 2, 2).intersection(Recti(0, 1, 2, 2)), Recti(1, 1, 1, 1));
}
