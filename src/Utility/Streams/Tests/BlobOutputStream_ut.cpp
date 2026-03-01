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

    // Write enough data to span multiple chunks (initial chunk is 1KB, then 2KB, 4KB, ...).
    std::string expected;
    for (int i = 0; i < 10000; i++) {
        std::string chunk = std::to_string(i) + " ";
        output.write(chunk.data(), chunk.size());
        expected += chunk;
    }

    output.close();
    EXPECT_EQ(blob.str(), expected);
}
