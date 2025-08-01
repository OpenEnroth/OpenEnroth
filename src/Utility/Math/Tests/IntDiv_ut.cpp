#include <climits>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Math/IntDiv.h"

UNIT_TEST(IntDiv, DivToNegInf) {
    EXPECT_EQ(divRemToNegInf(6, 3), std::pair(2, 0));
    EXPECT_EQ(divRemToNegInf(-6, 4), std::pair(-2, 2));

    EXPECT_EQ(divRemToNegInf(INT_MIN, INT_MAX), std::pair(-2, INT_MAX - 1));
}

UNIT_TEST(IntDiv, RoundToNegInf) {
    EXPECT_EQ(roundToNegInf(6, 3), 6);
    EXPECT_EQ(roundToNegInf(6, 4), 4);
    EXPECT_EQ(roundToNegInf(-6, 4), -8);
}
