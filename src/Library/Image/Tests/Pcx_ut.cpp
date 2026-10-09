#include "Testing/Unit/UnitTest.h"

#include "Library/Image/Pcx.h"

UNIT_TEST(Pcx, DecodeSize) {
    for (Sizei size : {Sizei(1, 1), Sizei(3, 2), Sizei(17, 64)}) {
        Blob data = pcx::encode(RgbaImage::solid(Color(10, 20, 30), size));
        EXPECT_EQ(pcx::decodeSize(data), size);
        EXPECT_EQ(pcx::decode(data).size(), size);
    }
}
