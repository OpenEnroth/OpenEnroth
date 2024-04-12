#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Streams/MemoryInputStream.h"

UNIT_TEST(InputStream, ReadAll) {
    std::string largeString(10000, 'a');
    MemoryInputStream input(largeString.data(), largeString.size());

    std::string resultingString = input.readAll();
    EXPECT_EQ(largeString, resultingString); // There was a bug in readAll resulting in failures on large files.
}
