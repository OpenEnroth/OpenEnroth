#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Streams/MemoryInputStream.h"

UNIT_TEST(MemoryInputStream, Seek) {
    MemoryInputStream input("abcdef", 6);
    EXPECT_EQ(input.position(), 0);

    input.seek(6);
    EXPECT_EQ(input.position(), 6);

    input.seek(10);
    EXPECT_EQ(input.position(), 6);
    EXPECT_EQ(input.readAll(), "");

    input.seek(3);
    EXPECT_EQ(input.readAll(), "def");
    EXPECT_EQ(input.position(), 6);
}
