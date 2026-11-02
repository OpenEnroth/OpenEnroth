#include "Testing/Unit/UnitTest.h"

#include "Library/Geometry/Vec.h"

UNIT_TEST(Vec, IntegerLengthOverflow) {
    // The length of an integer vector was computed from an int sum of squares. It overflowed above about 46000 units,
    // and the resulting NaN converted to a different int on x86 and on ARM.
    EXPECT_EQ(Vec2i(50000, 0).length(), 50000);
    EXPECT_EQ(Vec3i(50000, 0, 0).length(), 50000);
    EXPECT_EQ(Vec3i(30000, 0, 40000).length(), 50000);
}
