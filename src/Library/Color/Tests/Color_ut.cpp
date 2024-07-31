#include "Testing/Unit/UnitTest.h"

#include "Library/Color/Color.h"
#include "Library/Color/ColorTable.h"

#include "Utility/String/Format.h"

UNIT_TEST(Color, Tags) {
    EXPECT_THROW((void) fmt::format(fmt::runtime("{}"), colorTable.Anakiwa.tag()), std::exception);
    EXPECT_THROW((void) fmt::format(fmt::runtime("{:}"), colorTable.Anakiwa.tag()), std::exception);
    EXPECT_THROW((void) fmt::format(fmt::runtime("{: }"), colorTable.Anakiwa.tag()), std::exception);
    EXPECT_THROW((void) fmt::format(fmt::runtime("{:_}"), colorTable.Anakiwa.tag()), std::exception);
    EXPECT_THROW((void) fmt::format(fmt::runtime("{:!}"), colorTable.Anakiwa.tag()), std::exception);

    EXPECT_EQ(fmt::format("{::}", colorTable.White.tag()), "\f65535");
    EXPECT_EQ(fmt::format("{::}", colorTable.Black.tag()), "\f00000");
    EXPECT_EQ(fmt::format("{::}", colorTable.Blue.tag()), "\f00031");
}
