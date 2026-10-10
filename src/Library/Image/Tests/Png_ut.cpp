#include "Testing/Unit/UnitTest.h"

#include "Library/Image/Png.h"

UNIT_TEST(Png, DecodeSize) {
    // decodeSize() reads the same size from the header that decode() produces.
    for (Sizei size : {Sizei(1, 1), Sizei(3, 2), Sizei(17, 64)}) {
        Blob data = png::encode(RgbaImage::solid(Color(10, 20, 30), size));
        EXPECT_EQ(png::decodeSize(data), size);
        EXPECT_EQ(png::decode(data).size(), size);
    }
}
