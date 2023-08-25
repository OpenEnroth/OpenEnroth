#include <gtest/gtest.h>
#include <memory>
#include "Testing/Unit/UnitTest.h"
#include "Utility/Math/Float.h"

UNIT_TEST(Float, Fuzzy) {
    EXPECT_TRUE(fuzzyIsNull(0.000001f));
    EXPECT_FALSE(fuzzyIsNull(0.0001f));

    EXPECT_TRUE(fuzzyIsNull(0.0000000000001));
    EXPECT_FALSE(fuzzyIsNull(0.00000000001));

    EXPECT_TRUE(fuzzyEquals(1.0f, 1.0000001f));
    EXPECT_FALSE(fuzzyEquals(10000.0f, 10001.0f));

    EXPECT_TRUE(fuzzyEquals(1.0f, 1.0f));
    EXPECT_TRUE(fuzzyEquals(1.0, 1.0));
    EXPECT_FALSE(fuzzyEquals(1.0f, 2.0f));
    EXPECT_FALSE(fuzzyEquals(1.0, 2.0));
}
