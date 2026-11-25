#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Library/Compression/Compression.h"

#include "Utility/Exception.h"
#include "Utility/Memory/Blob.h"

static Blob makeCorruptZlibBlob(const Blob &source) {
    // Compress, then corrupt the last 4 bytes (Adler-32 checksum).
    Blob compressed = zlib::compress(source);
    Blob corrupted = Blob::copy(compressed.data(), compressed.size());
    auto *bytes = static_cast<uint8_t *>(const_cast<void *>(corrupted.data()));
    bytes[corrupted.size() - 1] ^= 0xFF; // Flip bits in the last byte.
    bytes[corrupted.size() - 2] ^= 0xFF;
    return corrupted;
}

UNIT_TEST(Compression, UncompressBestEffort_CorruptChecksum) {
    // Data that compresses and decompresses cleanly.
    std::string original(1000, 'A');
    Blob source = Blob::fromString(original);

    // Normal uncompress works.
    Blob compressed = zlib::compress(source);
    Blob decompressed = zlib::uncompress(compressed, source.size());
    EXPECT_EQ(decompressed.size(), source.size());

    // Corrupt the checksum — strict uncompress throws, best-effort recovers.
    Blob corrupted = makeCorruptZlibBlob(source);
    EXPECT_THROW(zlib::uncompress(corrupted, source.size()), Exception);

    Blob recovered = zlib::uncompressBestEffort(corrupted, source.size());
    ASSERT_EQ(recovered.size(), source.size());
    EXPECT_EQ(std::string_view(static_cast<const char *>(recovered.data()), recovered.size()), original);
}

UNIT_TEST(Compression, UncompressBestEffort_EmptyReturnsEmpty) {
    EXPECT_FALSE(zlib::uncompressBestEffort(Blob()));
}

UNIT_TEST(Compression, Detect) {
    // detect accepts the headers zlib writes, and rejects the ones zlib can't decode or that fail their checksum.
    EXPECT_TRUE(zlib::detect(zlib::compress(Blob::fromString(std::string(1000, 'A')))));
    EXPECT_TRUE(zlib::detect(Blob::fromString("\x78\x01")));
    EXPECT_TRUE(zlib::detect(Blob::fromString("\x78\xDA")));

    EXPECT_FALSE(zlib::detect(Blob()));
    EXPECT_FALSE(zlib::detect(Blob::fromString("\x78")));
    EXPECT_FALSE(zlib::detect(Blob::fromString("\x78\x9D"))); // Header checksum fails.
    EXPECT_FALSE(zlib::detect(Blob::fromString("\x7F\x07"))); // Method 15, reserved.
    EXPECT_FALSE(zlib::detect(Blob::fromString("\x88\x1C"))); // 64K window.
    EXPECT_FALSE(zlib::detect(Blob::fromString("\x78\x20"))); // Preset dictionary.
}
