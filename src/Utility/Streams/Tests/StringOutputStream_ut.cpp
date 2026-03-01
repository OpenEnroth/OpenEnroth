#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Streams/StringOutputStream.h"

UNIT_TEST(StringOutputStream, OpenClearsTarget) {
    std::string target = "old data";
    StringOutputStream output(&target);
    output.write("new");
    output.close();
    EXPECT_EQ(target, "new");
}

UNIT_TEST(StringOutputStream, DestructorFlushesData) {
    std::string target;
    {
        StringOutputStream output(&target);
        output.write("hello");
        output.write("world");
    }
    EXPECT_EQ(target, "helloworld");
}
