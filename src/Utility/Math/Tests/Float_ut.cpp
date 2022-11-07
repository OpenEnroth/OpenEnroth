#include "test/test.h"

#include "src/Utility/Math/Float.h"

GTEST(Float, Fuzzy) {
    EXPECT_TRUE(FuzzyIsNull(0.000001f));
    EXPECT_FALSE(FuzzyIsNull(0.0001f));

    EXPECT_TRUE(FuzzyIsNull(0.0000000000001));
    EXPECT_FALSE(FuzzyIsNull(0.00000000001));

    EXPECT_TRUE(FuzzyEquals(1.0f, 1.0000001f));
    EXPECT_FALSE(FuzzyEquals(10000.0f, 10001.0f));

    EXPECT_TRUE(FuzzyEquals(1.0f, 1.0f));
    EXPECT_TRUE(FuzzyEquals(1.0, 1.0));
    EXPECT_FALSE(FuzzyEquals(1.0f, 2.0f));
    EXPECT_FALSE(FuzzyEquals(1.0, 2.0));
}
