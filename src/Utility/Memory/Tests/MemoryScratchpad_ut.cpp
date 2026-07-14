#include <cstring>
#include <span>
#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Memory/MemoryScratchpad.h"

UNIT_TEST(MemoryScratchpad, Empty) {
    MemoryScratchpad scratchpad;
    EXPECT_TRUE(scratchpad.finish(0).empty());
}

UNIT_TEST(MemoryScratchpad, SingleChunk) {
    MemoryScratchpad scratchpad;

    std::span<char> chunk = scratchpad.next();
    const char *data = chunk.data();
    memcpy(chunk.data(), "abcd", 4);

    Blob blob = scratchpad.finish(4);
    EXPECT_EQ(blob.str(), "abcd");
    EXPECT_EQ(blob.data(), data); // Single chunk should be transferred into the Blob without copying.
}

UNIT_TEST(MemoryScratchpad, MultipleChunks) {
    MemoryScratchpad scratchpad;

    std::string expected;
    size_t bytesTotal = 0;
    for (int i = 0; i < 3; i++) {
        std::span<char> chunk = scratchpad.next();
        memset(chunk.data(), 'a' + i, chunk.size());

        size_t used = (i == 2) ? chunk.size() / 2 : chunk.size(); // Leave the last chunk partially filled.
        expected += std::string(used, 'a' + i);
        bytesTotal += used;
    }

    std::string copied(bytesTotal, '\0');
    scratchpad.materialize(copied.data(), bytesTotal);
    EXPECT_EQ(copied, expected);

    Blob blob = scratchpad.finish(bytesTotal);
    EXPECT_EQ(blob.str(), expected);
}

UNIT_TEST(MemoryScratchpad, ChunksGrow) {
    MemoryScratchpad scratchpad(4096);

    std::span<char> chunk = scratchpad.next();
    for (int i = 0; i < 10; i++) {
        std::span<char> next = scratchpad.next();
        EXPECT_GE(next.size(), chunk.size());
        EXPECT_LE(next.size(), 4096);
        chunk = next;
    }
    EXPECT_EQ(chunk.size(), 4096);
}

UNIT_TEST(MemoryScratchpad, MinSize) {
    MemoryScratchpad scratchpad;
    EXPECT_GE(scratchpad.next(10 * 1024 * 1024).size(), 10 * 1024 * 1024);
}
