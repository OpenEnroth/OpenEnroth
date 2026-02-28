#include <string>

#include "Utility/String/Format.h"

#include "Testing/Unit/UnitTest.h"

#include "Library/Snapshots/CommonSnapshots.h"
#include "Library/Vid/VidReader.h"
#include "Library/Vid/VidSnapshots.h"

#include "Utility/Streams/BlobOutputStream.h"

static Blob makeVidBlob(int entryCount) {
    // VID layout: uint32_t entryCount, then entryCount * VidEntry_MM7, then data.
    // Entry sizes are implied by the distance between consecutive offsets.
    size_t headerSize = 4 + entryCount * sizeof(VidEntry_MM7);
    size_t dataSize = 100;

    Blob result;
    BlobOutputStream stream(&result);

    serialize(uint32_t(entryCount), &stream);

    for (int i = 0; i < entryCount; i++) {
        VidEntry_MM7 entry = {};
        snapshot(fmt::format("video{}.smk", i), &entry.name);
        entry.offset = headerSize + i * dataSize;
        serialize(entry, &stream);
    }

    // Append dummy data.
    stream.write(std::string(entryCount * dataSize, '\0'));
    stream.close();
    return result;
}

UNIT_TEST(VidDetect, ValidVid) {
    EXPECT_TRUE(vid::detect(makeVidBlob(3)));
}

UNIT_TEST(VidDetect, EmptyBlob) {
    EXPECT_FALSE(vid::detect(Blob()));
}

UNIT_TEST(VidDetect, ZeroEntries) {
    Blob result;
    BlobOutputStream stream(&result);
    serialize(uint32_t(0), &stream);
    stream.close();
    EXPECT_FALSE(vid::detect(result));
}

UNIT_TEST(VidDetect, TruncatedHeader) {
    // Claim 1000 entries but provide only the count.
    Blob result;
    BlobOutputStream stream(&result);
    serialize(uint32_t(1000), &stream);
    stream.close();
    EXPECT_FALSE(vid::detect(result));
}

UNIT_TEST(VidDetect, OffsetInsideHeader) {
    Blob result;
    BlobOutputStream stream(&result);

    serialize(uint32_t(1), &stream);

    VidEntry_MM7 entry = {};
    snapshot(std::string("test.smk"), &entry.name);
    entry.offset = 0; // Points inside header.
    serialize(entry, &stream);

    stream.write(std::string(100, '\0'));
    stream.close();
    EXPECT_FALSE(vid::detect(result));
}

UNIT_TEST(VidDetect, NonMonotonicOffsets) {
    size_t headerSize = 4 + 2 * sizeof(VidEntry_MM7);

    Blob result;
    BlobOutputStream stream(&result);

    serialize(uint32_t(2), &stream);

    VidEntry_MM7 entry0 = {};
    snapshot(std::string("a.smk"), &entry0.name);
    entry0.offset = headerSize + 100;
    serialize(entry0, &stream);

    VidEntry_MM7 entry1 = {};
    snapshot(std::string("b.smk"), &entry1.name);
    entry1.offset = headerSize; // Less than entry0, non-monotonic.
    serialize(entry1, &stream);

    stream.write(std::string(200, '\0'));
    stream.close();
    EXPECT_FALSE(vid::detect(result));
}
