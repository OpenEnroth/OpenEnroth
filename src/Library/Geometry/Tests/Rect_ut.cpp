#include <memory>
#include <utility>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/Geometry/Rect.h"

UNIT_TEST(Rect, ContainsPoint) {
    EXPECT_FALSE(Recti().contains(Pointi())); // Empty rect contains nothing.

    EXPECT_TRUE(Recti(0, 0, 1, 1).contains(Pointi(0, 0)));
    EXPECT_FALSE(Recti(0, 0, 1, 1).contains(Pointi(0, 1)));
    EXPECT_FALSE(Recti(0, 0, 1, 1).contains(Pointi(1, 0)));
    EXPECT_FALSE(Recti(0, 0, 1, 1).contains(Pointi(1, 1)));

    EXPECT_TRUE(Recti(0, 0, 10, 10).contains(Pointi(5, 5)));

    EXPECT_TRUE(Recti(0, 0, 10, 10).contains(Pointi(0, 5)));
    EXPECT_TRUE(Recti(0, 0, 10, 10).contains(Pointi(5, 0)));
    EXPECT_FALSE(Recti(0, 0, 10, 10).contains(Pointi(10, 5)));
    EXPECT_FALSE(Recti(0, 0, 10, 10).contains(Pointi(5, 10)));
}

UNIT_TEST(Rect, ContainsRect) {
    EXPECT_TRUE(Recti().contains(Recti())); // Empty rect contains itself.
    EXPECT_TRUE(Recti(0, 0, 10, 10).contains(Recti(0, 0, 10, 10))); // Non-empty rect contains itself.

    EXPECT_TRUE(Recti(0, 0, 10, 10).contains(Recti(2, 2, 4, 4))); // Rect fully inside.
    EXPECT_TRUE(Recti(0, 0, 10, 10).contains(Recti(5, 5, 0, 0))); // Zero-side rect fully inside.

    EXPECT_FALSE(Recti(0, 0, 10, 10).contains(Recti(11, 11, 5, 5))); // Rect outside.
    EXPECT_FALSE(Recti(0, 0, 10, 10).contains(Recti(8, 8, 5, 5))); // Rects overlap.
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
