#include <cstdint>
#include <utility>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/Binary/CommonSerialization.h"

#include "Utility/Streams/BlobOutputStream.h"
#include "Utility/Streams/BlobInputStream.h"

UNIT_TEST(Binary, GarbageSizeThrowsInsteadOfBadAlloc) {
    // Craft a stream with a garbage uint32_t size prefix that claims more elements than the stream contains.
    // This should throw a descriptive serialization error, not std::bad_alloc.
    Blob blob;
    BlobOutputStream out(&blob);
    uint32_t garbageSize = 2'000'000'000; // Claims 2B ints, but stream only has 4+4=8 bytes total.
    int oneElement = 42;
    out.write(&garbageSize, sizeof(garbageSize));
    out.write(&oneElement, sizeof(oneElement));
    out.close();

    BlobInputStream input(std::move(blob));
    std::vector<int> dst;
    EXPECT_THROW_MESSAGE(deserialize(input, &dst), "expected");
}
