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

UNIT_TEST(StringOutputStream, CloseWithoutWriting) {
    std::string target = "old";
    StringOutputStream output(&target);
    output.close();
    EXPECT_EQ(target, "");
}

UNIT_TEST(StringOutputStream, FlushMidStream) {
    std::string target;
    StringOutputStream output(&target);
    output.write("hello");
    output.flush();
    EXPECT_EQ(target, "hello");

    output.write(" world");
    output.close();
    EXPECT_EQ(target, "hello world");
}

UNIT_TEST(StringOutputStream, CloseIdempotent) {
    std::string target;
    StringOutputStream output(&target);
    output.write("hello");
    output.close();
    EXPECT_FALSE(output.isOpen());
    EXPECT_NO_THROW(output.close()); // Double close is fine.
    EXPECT_FALSE(output.isOpen());
    EXPECT_EQ(target, "hello");
}

UNIT_TEST(StringOutputStream, ReopenAfterClose) {
    std::string target;
    StringOutputStream output(&target);
    output.write("first");
    output.close();
    EXPECT_EQ(target, "first");

    output.open(&target);
    output.write("second");
    output.close();
    EXPECT_EQ(target, "second");
}

UNIT_TEST(StringOutputStream, LargeWrite) {
    std::string target;
    StringOutputStream output(&target);

    std::string large(8192, 'x');
    output.write(large.data(), large.size());
    output.close();
    EXPECT_EQ(target, large);
}

UNIT_TEST(StringOutputStream, GrowthCap) {
    // Chunks grow geometrically (1KB, 2KB, ..., 1MB cap). Write ~3MB to hit the cap.
    std::string target;
    StringOutputStream output(&target);

    std::string expected;
    std::string chunk(1024, 'a');
    for (int i = 0; i < 3072; i++) {
        chunk[0] = static_cast<char>('a' + (i % 26));
        output.write(chunk.data(), chunk.size());
        expected.append(chunk);
    }
    output.close();
    EXPECT_EQ(target, expected);
}

UNIT_TEST(StringOutputStream, MultipleChunks) {
    std::string target;
    StringOutputStream output(&target);

    std::string expected;
    for (int i = 0; i < 10000; i++) {
        std::string chunk = std::to_string(i) + " ";
        output.write(chunk.data(), chunk.size());
        expected += chunk;
    }

    output.close();
    EXPECT_EQ(target, expected);
}

UNIT_TEST(StringOutputStream, WriteZero) {
    std::string target;
    StringOutputStream output(&target);
    output.write("hello");
    output.write(nullptr, 0);
    output.write(" world");
    output.close();
    EXPECT_EQ(target, "hello world");
}
