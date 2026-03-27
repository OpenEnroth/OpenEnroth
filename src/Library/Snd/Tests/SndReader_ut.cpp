#include <string>
#include <utility>

#include "Utility/String/Format.h"

#include "Testing/Unit/UnitTest.h"

#include "Library/Compression/Compression.h"
#include "Library/Logger/BufferLogSink.h"
#include "Library/Logger/Logger.h"
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

static Blob makeSndBlobWithCorruptCompressedEntry(std::string_view content) {
    // Compress content, then corrupt the Adler-32 checksum.
    Blob compressed = zlib::compress(Blob::fromString(std::string(content)));
    auto *bytes = static_cast<uint8_t *>(const_cast<void *>(compressed.data()));
    bytes[compressed.size() - 1] ^= 0xFF;
    bytes[compressed.size() - 2] ^= 0xFF;

    size_t headerSize = 4 + sizeof(SndEntry_MM7);
    Blob result;
    BlobOutputStream stream(&result);

    serialize(uint32_t(1), &stream);

    SndEntry_MM7 entry = {};
    snapshot(std::string("corrupt.wav"), &entry.name);
    entry.offset = headerSize;
    entry.size = compressed.size();
    entry.decompressedSize = content.size(); // deliberately wrong checksum in payload
    serialize(entry, &stream);

    stream.write(compressed);
    stream.close();
    return result;
}

UNIT_TEST(SndReader, CorruptChecksumReturnsPartialData) {
    // SND entries with a corrupt zlib checksum should return recovered data, not an empty blob.
    // This mirrors the real-world case of 02Flame01.wav in the GOG version of MM7.
    BufferLogSink sink;
    Logger testLogger(LOG_TRACE, &sink);

    std::string original(1000, 'B');
    Blob snd = makeSndBlobWithCorruptCompressedEntry(original);

    SndReader reader(std::move(snd));
    Blob result = reader.read("corrupt.wav");
    ASSERT_EQ(result.size(), original.size());
    EXPECT_EQ(std::string_view(static_cast<const char *>(result.data()), result.size()), original);
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
