#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Memory/Blob.h"
#include "Utility/Streams/BlobOutputStream.h"

UNIT_TEST(BlobOutputStream, DestructorFlushesData) {
    Blob blob;
    {
        BlobOutputStream output(&blob);
        output.write("hello");
        output.write("world");
    }
    EXPECT_EQ(blob.str(), "helloworld");
}

UNIT_TEST(BlobOutputStream, MultipleChunks) {
    Blob blob;
    BlobOutputStream output(&blob);

    std::string expected;
    for (int i = 0; i < 10000; i++) {
        std::string chunk = std::to_string(i) + " ";
        output.write(chunk.data(), chunk.size());
        expected += chunk;
    }

    output.close();
    EXPECT_EQ(blob.str(), expected);
}

UNIT_TEST(BlobOutputStream, CloseWithoutWriting) {
    Blob blob = Blob::fromString("old");
    BlobOutputStream output(&blob);
    output.close();
    EXPECT_EQ(blob.size(), 0u);
}

UNIT_TEST(BlobOutputStream, FlushMidStream) {
    Blob blob;
    BlobOutputStream output(&blob);
    output.write("hello");
    output.flush();
    EXPECT_EQ(blob.str(), "hello");

    output.write(" world");
    output.close();
    EXPECT_EQ(blob.str(), "hello world");
}

UNIT_TEST(BlobOutputStream, DisplayPath) {
    Blob blob;
    BlobOutputStream output(&blob, "test.bin");
    output.write("data");
    output.close();
    EXPECT_EQ(blob.displayPath(), "test.bin");
}

UNIT_TEST(BlobOutputStream, DisplayPathEmptyStream) {
    Blob blob;
    BlobOutputStream output(&blob, "empty.bin");
    output.close();
    EXPECT_EQ(blob.displayPath(), "empty.bin");
}

UNIT_TEST(BlobOutputStream, CloseIdempotent) {
    Blob blob;
    BlobOutputStream output(&blob);
    output.write("hello");
    output.close();
    EXPECT_FALSE(output.isOpen());
    EXPECT_NO_THROW(output.close()); // Double close is fine.
    EXPECT_FALSE(output.isOpen());
    EXPECT_EQ(blob.str(), "hello");
}

UNIT_TEST(BlobOutputStream, ReopenAfterClose) {
    Blob blob;
    BlobOutputStream output(&blob);
    output.write("first");
    output.close();
    EXPECT_EQ(blob.str(), "first");

    output.open(&blob);
    output.write("second");
    output.close();
    EXPECT_EQ(blob.str(), "second");
}

UNIT_TEST(BlobOutputStream, LargeWrite) {
    Blob blob;
    BlobOutputStream output(&blob);

    std::string large(8192, 'x');
    output.write(large.data(), large.size());
    output.close();
    EXPECT_EQ(blob.str(), large);
}

UNIT_TEST(BlobOutputStream, GrowthCap) {
    // Chunks grow geometrically (1KB, 2KB, ..., 1MB cap). Write ~3MB to hit the cap.
    Blob blob;
    BlobOutputStream output(&blob);

    std::string expected;
    std::string chunk(1024, 'a');
    for (int i = 0; i < 3072; i++) {
        chunk[0] = static_cast<char>('a' + (i % 26));
        output.write(chunk.data(), chunk.size());
        expected.append(chunk);
    }
    output.close();
    EXPECT_EQ(blob.str(), expected);
}

UNIT_TEST(BlobOutputStream, WriteZero) {
    Blob blob;
    BlobOutputStream output(&blob);
    output.write("hello");
    output.write(nullptr, 0);
    output.write(" world");
    output.close();
    EXPECT_EQ(blob.str(), "hello world");
}

UNIT_TEST(BlobOutputStream, PositionStartsAtZero) {
    Blob blob;
    BlobOutputStream output(&blob);
    EXPECT_EQ(output.position(), 0u);
    output.close();
}

UNIT_TEST(BlobOutputStream, PositionAdvancesOnWrite) {
    Blob blob;
    BlobOutputStream output(&blob);
    output.write("hello");
    EXPECT_EQ(output.position(), 5u);
    output.write(" world");
    EXPECT_EQ(output.position(), 11u);
    output.close();
}

UNIT_TEST(BlobOutputStream, PositionAfterFlush) {
    Blob blob;
    BlobOutputStream output(&blob);
    output.write("hello");
    output.flush();
    EXPECT_EQ(output.position(), 5u);
    output.write(" world");
    EXPECT_EQ(output.position(), 11u);
    output.close();
}

UNIT_TEST(BlobOutputStream, PositionAfterLargeWrite) {
    Blob blob;
    BlobOutputStream output(&blob);
    std::string large(8192, 'x');
    output.write(large);
    EXPECT_EQ(output.position(), 8192u);
    output.close();
}

UNIT_TEST(BlobOutputStream, PositionResetsOnReopen) {
    Blob blob;
    BlobOutputStream output(&blob);
    output.write("hello");
    EXPECT_EQ(output.position(), 5u);
    output.close();

    output.open(&blob);
    EXPECT_EQ(output.position(), 0u);
    output.close();
}
