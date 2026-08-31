#include "Testing/Unit/UnitTest.h"

#include "Library/Geometry/Vec.h"

// Octagonal length is an approximation, and the gameplay code depends on the error. Swapping in a real Euclidean
// length silently shifts AI aggro ranges, light attenuation and the minimap, so these values are pinned.
UNIT_TEST(Vec, OctagonalLength) {
    EXPECT_EQ(Vec3i(0, 0, 0).octagonalLength(), 0);
    EXPECT_EQ(Vec3i(10, 0, 0).octagonalLength(), 10);
    EXPECT_EQ(Vec3i(0, 0, 8).octagonalLength(), 8);

    EXPECT_EQ(Vec3i(10, 10, 10).octagonalLength(), 15); // Euclidean length is 17.
    EXPECT_EQ(Vec3i(100, 100, 0).octagonalLength(), 134); // Euclidean length is 141.
    EXPECT_EQ(Vec3i(300, 400, 0).octagonalLength(), 503); // Overshoots, the Euclidean length is 500.

    EXPECT_EQ(Vec2i(0, 0).octagonalLength(), 0);
    EXPECT_EQ(Vec2i(100, 100).octagonalLength(), 134);
    EXPECT_EQ(Vec2i(300, 400).octagonalLength(), 503);
}

// Every call site used to pass std::abs of each component, and folding that into octagonalLength only stayed
// behavior preserving because the sign is dropped before the components are sorted.
UNIT_TEST(Vec, OctagonalLengthIsSignAgnostic) {
    EXPECT_EQ(Vec3i(-10, -10, -10).octagonalLength(), 15);
    EXPECT_EQ(Vec3i(-300, 400, 0).octagonalLength(), 503);
    EXPECT_EQ(Vec2i(300, -400).octagonalLength(), 503);
}

// The two overloads share no code, so a change to one can drift away from the other.
UNIT_TEST(Vec, OctagonalLengthAgreesIn2dAnd3d) {
    for (int x = -40; x <= 40; x += 7)
        for (int y = -40; y <= 40; y += 7)
            EXPECT_EQ(Vec2i(x, y).octagonalLength(), Vec3i(x, y, 0).octagonalLength());
}
