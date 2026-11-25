#include <algorithm>
#include <array>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/Compression/Compression.h"
#include "Library/LodFormats/LodFormats.h"
#include "Library/LodFormats/LodFormatSnapshots.h"
#include "Utility/Memory/Blob.h"

// Builds a blob in MM8's `EnglishT.lod` per-entry format: a `LodCompressionHeader_MM8` (with all the "always
// zero"/"always 256" fields set to their observed real-data values) followed by a zlib-compressed payload.
static Blob makeCompressedBlobMm8(std::string_view name, std::string_view payload) {
    LodCompressionHeader_MM8 header = {};
    std::ranges::copy(name, header.name.begin());
    Blob compressed = zlib::compress(Blob::view(payload));
    header.dataSize = static_cast<std::uint32_t>(compressed.size());
    header.decompressedSize = static_cast<std::uint32_t>(payload.size());
    header.unk_15 = 256;

    return Blob::concat(Blob::view(&header, sizeof(header)), compressed);
}

// Builds a raw `LodImageHeader_MM6`-shaped blob (48 bytes) followed by a 768-byte (0x300) raw RGB palette, i.e. a
// bare LOD palette entry. `paletteId` is placed at offset 36 (name[16] + size(4) + dataSize(4) + width(2) +
// height(2) + widthLn2(2) + heightLn2(2) + widthMinus1(2) + heightMinus1(2)).
static Blob makeBarePaletteBlob(std::string_view name, int16_t paletteId, std::uint8_t firstColorByte) {
    std::array<std::uint8_t, 48 + 0x300> bytes{};
    std::ranges::copy(name, bytes.begin());
    bytes[36] = static_cast<std::uint8_t>(paletteId & 0xFF);
    bytes[37] = static_cast<std::uint8_t>((paletteId >> 8) & 0xFF);
    bytes[48] = firstColorByte; // First byte of the raw palette data, so we can check round-tripping.
    return Blob::copy(bytes.data(), bytes.size());
}

// Builds a blob in MM8's `EnglishD.lod` image entry format (`LodImageHeader_MM8`): the header, followed by
// zlib-compressed pixel data, followed by a raw 768-byte (0x300) palette.
static Blob makeImageBlobMm8(std::string_view name, int width, int height, const std::vector<std::uint8_t> &pixels) {
    LodImageHeader_MM8 header = {};
    std::ranges::copy(name, header.name.begin());
    header.size = static_cast<std::uint32_t>(width * height);
    header.width = static_cast<std::uint16_t>(width);
    header.height = static_cast<std::uint16_t>(height);
    header.decompressedSize = static_cast<std::uint32_t>(pixels.size());

    Blob compressed = zlib::compress(Blob::view(pixels.data(), pixels.size()));
    header.dataSize = static_cast<std::uint32_t>(compressed.size());

    std::array<std::uint8_t, 0x300> palette{};

    return Blob::concat(Blob::concat(Blob::view(&header, sizeof(header)), compressed),
                         Blob::view(palette.data(), palette.size()));
}

// Builds a blob in MM8's `EnglishD.lod` compressed pseudo-image format: a `LodImageHeader_MM8` with
// `flags & 256` set and `size`/`width`/`height` all zero, followed by a zlib-compressed payload (a raw
// embedded file, e.g. a PCX image, rather than `LodImage` pixel data).
static Blob makePseudoImageBlobMm8(std::string_view name, std::string_view payload) {
    LodImageHeader_MM8 header = {};
    std::ranges::copy(name, header.name.begin());
    Blob compressed = zlib::compress(Blob::view(payload));
    header.dataSize = static_cast<std::uint32_t>(compressed.size());
    header.decompressedSize = static_cast<std::uint32_t>(payload.size());
    header.flags = 256;

    return Blob::concat(Blob::view(&header, sizeof(header)), compressed);
}

UNIT_TEST(LodFormats, DetectCompressedPseudoImageMm8RealTitlePcx) {
    // MM8's `EnglishD.lod` wraps `title.pcx` (a raw embedded PCX file, not `LodImage` pixel data) in
    // this format instead of `LodImageHeader_MM6`'s `decodeCompressedPseudoImage` shape.
    Blob blob = makePseudoImageBlobMm8("title.pcx", "\x0a\x05\x01\x08" "fake pcx bytes");

    EXPECT_TRUE(lod::detectCompressedPseudoImageMm8(blob));
}

UNIT_TEST(LodFormats, DecodeCompressedPseudoImageMm8RealTitlePcx) {
    Blob blob = makePseudoImageBlobMm8("title.pcx", "\x0a\x05\x01\x08" "fake pcx bytes");

    Blob decoded = lod::decodeCompressedPseudoImageMm8(blob);
    EXPECT_EQ(decoded.str(), "\x0a\x05\x01\x08" "fake pcx bytes");
}

UNIT_TEST(LodFormats, DecodeMaybeCompressedMm8PseudoImage) {
    // `decodeMaybeCompressed` should transparently unwrap this format too.
    Blob blob = makePseudoImageBlobMm8("title.pcx", "\x0a\x05\x01\x08" "fake pcx bytes");

    Blob decoded = lod::decodeMaybeCompressed(blob);
    EXPECT_EQ(decoded.str(), "\x0a\x05\x01\x08" "fake pcx bytes");
}

UNIT_TEST(LodFormats, DetectCompressedPseudoImageMm8RejectsRealImage) {
    // Regression check: an actual MM8 pixel-data image entry (nonzero width/height) doesn't falsely detect as
    // a compressed pseudo image.
    Blob blob = makeImageBlobMm8("t_new_up", 4, 2, {1, 2, 3, 4, 5, 6, 7, 8});

    EXPECT_FALSE(lod::detectCompressedPseudoImageMm8(blob));
}

UNIT_TEST(LodFormats, DetectImageMm8RealImage) {
    // MM8's `EnglishD.lod` main menu art (title.pcx, button textures like `t_new_up`) uses this format
    // instead of `LodImageHeader_MM6`'s.
    Blob blob = makeImageBlobMm8("t_new_up", 4, 2, {1, 2, 3, 4, 5, 6, 7, 8});

    bool isPalette = false;
    EXPECT_TRUE(lod::detectImageMm8(blob, &isPalette));
    EXPECT_FALSE(isPalette);
}

UNIT_TEST(LodFormats, DecodeImageMm8RealImage) {
    Blob blob = makeImageBlobMm8("t_new_up", 4, 2, {1, 2, 3, 4, 5, 6, 7, 8});

    LodImage image = lod::decodeImageMm8(blob);
    EXPECT_EQ(image.image.width(), 4);
    EXPECT_EQ(image.image.height(), 2);
    EXPECT_EQ(image.image[0][0], 1);
    EXPECT_EQ(image.image[1][3], 8);
}

UNIT_TEST(LodFormats, DetectImageMm8RejectsMm6Image) {
    // Regression check: a plain MM6/7-format image entry (no 48-byte padding after `name`) doesn't falsely
    // detect as MM8's image format.
    Blob blob = makeBarePaletteBlob("pal01", 0, 20);

    EXPECT_FALSE(lod::detectImageMm8(blob));
}

UNIT_TEST(LodFormats, DetectImageMm8BarePaletteWithNonZeroPaletteId) {
    // MM8's own bare palette entries (e.g. `pal857` in MM8's `bitmaps.lod`)
    // self-populate `paletteId` with their own numeric id instead of leaving it at zero like MM6/7 do.
    Blob blob = makeBarePaletteBlob("pal857", 857, 10);

    bool isPalette = false;
    EXPECT_TRUE(lod::detectImage(blob, &isPalette));
    EXPECT_TRUE(isPalette);
}

UNIT_TEST(LodFormats, DecodePaletteMm8BarePaletteWithNonZeroPaletteId) {
    Blob blob = makeBarePaletteBlob("pal857", 857, 10);

    Palette palette = lod::decodePalette(blob);
    EXPECT_EQ(palette.colors[0], Color(10, 0, 0));
}

UNIT_TEST(LodFormats, DetectImageMm6BarePaletteWithZeroPaletteId) {
    // Regression check: MM6/7 bare palette entries still detect fine with `paletteId == 0`.
    Blob blob = makeBarePaletteBlob("pal01", 0, 20);

    bool isPalette = false;
    EXPECT_TRUE(lod::detectImage(blob, &isPalette));
    EXPECT_TRUE(isPalette);
}

// Unlike MM6/7, where per-entry zlib compression is optional (flagged by `LodCompressionHeader_MM6`'s
// "mvii" signature), every entry in MM8's `EnglishT.lod` is unconditionally wrapped in this format - confirmed
// against all 194 entries of MM8's `EnglishT.lod`. Without this, `eventsData()` calls for
// MM8 (e.g. `MapStats.txt`, `trans.txt`) return raw compressed bytes instead of readable table data.
UNIT_TEST(LodFormats, DetectCompressedDataMm8) {
    Blob blob = makeCompressedBlobMm8("mapstats.txt", "hello, MM8");

    EXPECT_TRUE(lod::detectCompressedDataMm8(blob));
}

UNIT_TEST(LodFormats, DecodeCompressedDataMm8) {
    Blob blob = makeCompressedBlobMm8("mapstats.txt", "hello, MM8");

    Blob decoded = lod::decodeCompressedDataMm8(blob);
    EXPECT_EQ(decoded.str(), "hello, MM8");
}

UNIT_TEST(LodFormats, DecodeMaybeCompressedMm8) {
    Blob blob = makeCompressedBlobMm8("mapstats.txt", "hello, MM8");

    Blob decoded = lod::decodeMaybeCompressed(blob);
    EXPECT_EQ(decoded.str(), "hello, MM8");
}

UNIT_TEST(LodFormats, DetectCompressedDataMm8RejectsPlainData) {
    // Regression check: plain, uncompressed MM6/7-style data (e.g. `items.txt`) doesn't falsely detect as MM8's
    // compression format.
    Blob blob = Blob::view("Item #\tPic File\tName\r\n1\titem001\tLongsword\r\n");

    EXPECT_FALSE(lod::detectCompressedDataMm8(blob));
}

// MM8's `EnglishD.lod` has thousands of `.wav` sound sample entries whose full name
// (e.g. "StartMainChoice02.wav", 21 bytes) doesn't fit `LodCompressionHeader_MM8::name` (16 bytes) - the tail
// spills into the following `unk_0` field instead, so `unk_0` is not always zero.
// `unk_0`'s first few bytes are the name's overflow text
// ("2.wav\0"), zero-padded after that. `detectCompressedDataMm8` must still recognize these as compressed data
// instead of falling through to `decodeMaybeCompressed`'s "not compressed" passthrough (which fed raw header
// bytes to ffmpeg as if they were audio and crashed `AudioBaseDataSource`, see `AudioPlayer::LoadSound`).
UNIT_TEST(LodFormats, DetectCompressedDataMm8LongNameOverflowsIntoUnkZero) {
    LodCompressionHeader_MM8 header = {};
    constexpr std::string_view fullName = "StartMainChoice02.wav";
    static_assert(fullName.size() > header.name.size());
    std::ranges::copy_n(fullName.begin(), header.name.size(), header.name.begin());
    std::span<std::byte> unkBytes = std::as_writable_bytes(std::span(header.unk_0));
    std::ranges::copy(fullName.substr(header.name.size()), reinterpret_cast<char *>(unkBytes.data()));

    // MM8's `EnglishD.lod` `.wav` entries also consistently carry this fixed, nonzero value in
    // `unk_14[1]` - regression check that `detectCompressedDataMm8` doesn't require it to be zero.
    header.unk_14[1] = 0x001f0000;

    Blob compressed = zlib::compress(Blob::view("hello, MM8"));
    header.dataSize = static_cast<std::uint32_t>(compressed.size());
    header.decompressedSize = 10;
    header.unk_15 = 256;

    Blob blob = Blob::concat(Blob::view(&header, sizeof(header)), compressed);

    EXPECT_TRUE(lod::detectCompressedDataMm8(blob));
    EXPECT_EQ(lod::decodeMaybeCompressed(blob).str(), "hello, MM8");
}
