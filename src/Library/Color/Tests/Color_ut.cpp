#include <string>

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

UNIT_TEST(Color, DeserializeRGB) {
    Color result;

    // Valid RGB colors
    EXPECT_TRUE(tryDeserialize("#000000", &result));
    EXPECT_EQ(result, Color(0, 0, 0));

    EXPECT_TRUE(tryDeserialize("#ffffff", &result));
    EXPECT_EQ(result, Color(255, 255, 255));

    EXPECT_TRUE(tryDeserialize("#FFFFFF", &result));
    EXPECT_EQ(result, Color(255, 255, 255));

    EXPECT_TRUE(tryDeserialize("#ff00ff", &result));
    EXPECT_EQ(result, Color(255, 0, 255));

    EXPECT_TRUE(tryDeserialize("#123456", &result));
    EXPECT_EQ(result, Color(0x12, 0x34, 0x56));
}

UNIT_TEST(Color, DeserializeRGBA) {
    Color result;

    // Valid RGBA colors
    EXPECT_TRUE(tryDeserialize("#00000000", &result));
    EXPECT_EQ(result, Color(0, 0, 0, 0));

    EXPECT_TRUE(tryDeserialize("#ffffffff", &result));
    EXPECT_EQ(result, Color(255, 255, 255, 255));

    EXPECT_TRUE(tryDeserialize("#FFFFFFFF", &result));
    EXPECT_EQ(result, Color(255, 255, 255, 255));

    EXPECT_TRUE(tryDeserialize("#ff00ff80", &result));
    EXPECT_EQ(result, Color(255, 0, 255, 128));

    EXPECT_TRUE(tryDeserialize("#12345678", &result));
    EXPECT_EQ(result, Color(0x12, 0x34, 0x56, 0x78));
}

UNIT_TEST(Color, DeserializeInvalid) {
    Color result;

    // Empty string
    EXPECT_FALSE(tryDeserialize("", &result));

    // Missing #
    EXPECT_FALSE(tryDeserialize("ffffff", &result));
    EXPECT_FALSE(tryDeserialize("ffffffff", &result));

    // Wrong length
    EXPECT_FALSE(tryDeserialize("#", &result));
    EXPECT_FALSE(tryDeserialize("#f", &result));
    EXPECT_FALSE(tryDeserialize("#ff", &result));
    EXPECT_FALSE(tryDeserialize("#fff", &result));
    EXPECT_FALSE(tryDeserialize("#ffff", &result));
    EXPECT_FALSE(tryDeserialize("#fffff", &result));
    EXPECT_FALSE(tryDeserialize("#fffffff", &result));
    EXPECT_FALSE(tryDeserialize("#fffffffff", &result));

    // Invalid hex characters
    EXPECT_FALSE(tryDeserialize("#gggggg", &result));
    EXPECT_FALSE(tryDeserialize("#12345g", &result));
    EXPECT_FALSE(tryDeserialize("#1234567g", &result));
    EXPECT_FALSE(tryDeserialize("#zzzzzz", &result));

    // Special characters
    EXPECT_FALSE(tryDeserialize("#12-456", &result));
    EXPECT_FALSE(tryDeserialize("#12 456", &result));
}

UNIT_TEST(Color, SerializeDeserializeRoundTrip) {
    std::string serialized;
    Color result;

    // RGB color (alpha = 255)
    Color rgb(0x12, 0x34, 0x56);
    EXPECT_TRUE(trySerialize(rgb, &serialized));
    EXPECT_TRUE(tryDeserialize(serialized, &result));
    EXPECT_EQ(result, rgb);

    // RGBA color (alpha != 255)
    Color rgba(0x12, 0x34, 0x56, 0x78);
    EXPECT_TRUE(trySerialize(rgba, &serialized));
    EXPECT_TRUE(tryDeserialize(serialized, &result));
    EXPECT_EQ(result, rgba);
}
