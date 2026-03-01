#include <string>

#include "Utility/String/Format.h"

#include "Testing/Unit/UnitTest.h"

#include "Library/Snapshots/CommonSnapshots.h"
#include "Library/Snd/SndReader.h"
#include "Library/Snd/SndSnapshots.h"

#include "Utility/Streams/BlobOutputStream.h"

static Blob makeSndBlob(int entryCount) {
    // SND layout: uint32_t entryCount, then entryCount * SndEntry_MM7, then data.
    size_t headerSize = 4 + entryCount * sizeof(SndEntry_MM7);
    size_t dataSize = 100;

    Blob result;
    BlobOutputStream stream(&result);

    serialize(uint32_t(entryCount), &stream);

    for (int i = 0; i < entryCount; i++) {
        SndEntry_MM7 entry = {};
        snapshot(fmt::format("sound{}.wav", i), &entry.name);
        entry.offset = headerSize + i * dataSize;
        entry.size = dataSize;
        entry.decompressedSize = 0; // Uncompressed.
        serialize(entry, &stream);
    }

    // Append dummy data.
    stream.write(std::string(entryCount * dataSize, '\0'));
    stream.close();
    return result;
}

UNIT_TEST(SndDetect, ValidSnd) {
    EXPECT_TRUE(snd::detect(makeSndBlob(3)));
}

UNIT_TEST(SndDetect, EmptyBlob) {
    EXPECT_FALSE(snd::detect(Blob()));
}

UNIT_TEST(SndDetect, ZeroEntries) {
    Blob result;
    BlobOutputStream stream(&result);
    serialize(uint32_t(0), &stream);
    stream.close();
    EXPECT_FALSE(snd::detect(result));
}

UNIT_TEST(SndDetect, TruncatedHeader) {
    // Claim 1000 entries but provide only the count.
    Blob result;
    BlobOutputStream stream(&result);
    serialize(uint32_t(1000), &stream);
    stream.close();
    EXPECT_FALSE(snd::detect(result));
}

UNIT_TEST(SndDetect, OffsetInsideHeader) {
    Blob result;
    BlobOutputStream stream(&result);

    serialize(uint32_t(1), &stream);

    SndEntry_MM7 entry = {};
    snapshot(std::string("test.wav"), &entry.name);
    entry.offset = 0; // Points inside header.
    entry.size = 10;
    entry.decompressedSize = 0;
    serialize(entry, &stream);

    stream.write(std::string(100, '\0'));
    stream.close();
    EXPECT_FALSE(snd::detect(result));
}

UNIT_TEST(SndDetect, GarbageDecompressedSize) {
    size_t headerSize = 4 + sizeof(SndEntry_MM7);

    Blob result;
    BlobOutputStream stream(&result);

    serialize(uint32_t(1), &stream);

    SndEntry_MM7 entry = {};
    snapshot(std::string("test.wav"), &entry.name);
    entry.offset = headerSize;
    entry.size = 100;
    entry.decompressedSize = 0x776B656B; // "kekw" as a little-endian int - obvious garbage.
    serialize(entry, &stream);

    stream.write(std::string(100, '\0'));
    stream.close();
    EXPECT_FALSE(snd::detect(result));
}

UNIT_TEST(SndDetect, OffsetPastEnd) {
    Blob result;
    BlobOutputStream stream(&result);

    serialize(uint32_t(1), &stream);

    SndEntry_MM7 entry = {};
    snapshot(std::string("test.wav"), &entry.name);
    entry.offset = 4 + sizeof(SndEntry_MM7);
    entry.size = 9999; // Way past end.
    entry.decompressedSize = 0;
    serialize(entry, &stream);

    stream.close();
    EXPECT_FALSE(snd::detect(result));
}
