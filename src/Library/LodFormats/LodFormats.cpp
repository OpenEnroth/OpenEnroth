#include "LodFormats.h"

#include <cassert>
#include <cstdint>
#include <span>
#include <vector>
#include <string>
#include <algorithm>
#include <ranges>

#include "LodFormatSnapshots.h"

#include "Library/Binary/CommonSerialization.h"
#include "Library/Snapshots/CommonSnapshots.h"
#include "Library/Compression/Compression.h"
#include "Library/Serialization/EnumSerialization.h"
#include "Library/Snapshots/SnapshotSerialization.h"

#include "Utility/Streams/MemoryInputStream.h"
#include "Utility/Streams/BlobInputStream.h"
#include "Utility/Memory/Blob.h"
#include "Utility/Exception.h"
#include "Utility/Lambda.h"

enum {
    MIN_GLYPH_WIDTH = 1,
    MAX_GLYPH_WIDTH = 63,
    MIN_GLYPH_HEIGHT = 4,
    MAX_GLYPH_HEIGHT = 63,
    MAX_GLYPH_SPACING = 63,
};

static void deserialize(InputStream &src, Palette *dst) {
    std::array<std::uint8_t, 0x300> rawPalette;
    src.readOrFail(rawPalette.data(), rawPalette.size());

    for (size_t i = 0; i < 256; i++)
        dst->colors[i] = Color(rawPalette[i * 3], rawPalette[i * 3 + 1], rawPalette[i * 3 + 2]);
}

/**
 * Inflates the zlib data of a LOD entry. A broken adler32 checksum is tolerated, since some entries carry one over
 * intact data, and the unpacked size from the entry's header takes its place as the integrity check.
 *
 * @param blob                          LOD entry the data comes from, for error messages.
 * @param compressed                    zlib data.
 * @param decompressedSize              Unpacked size, as the entry's header gives it.
 * @return                              Unpacked data.
 * @throw Exception                     If the data doesn't unpack into exactly `decompressedSize` bytes.
 */
static Blob uncompressEntryData(const Blob &blob, const Blob &compressed, size_t decompressedSize) {
    Blob result = zlib::uncompressBestEffort(compressed, decompressedSize);
    if (result.size() != decompressedSize)
        throw Exception("Cannot decode LOD entry '{}', its data unpacks into {} bytes instead of {}", blob.displayPath(), result.size(), decompressedSize);
    return result.withDisplayPath(blob.displayPath());
}

/**
 * Reads and checks the header both LOD font layouts share.
 *
 * @param stream                        Stream at the start of the font. It's left past the header.
 * @param blobSize                      Size of the whole font entry.
 * @param[out] header                   Font header.
 * @return                              Whether the header is a valid LOD font header.
 */
static bool detectFontHeader(MemoryInputStream &stream, size_t blobSize, LodFontHeader_MM6 *header) {
    if (blobSize < sizeof(LodFontHeader_MM6))
        return false;

    deserialize(stream, header);
    return header->firstChar < header->lastChar && header->field_3 == 8 && header->field_4 == 0 && header->field_5 == 0 &&
        header->height >= MIN_GLYPH_HEIGHT && header->height <= MAX_GLYPH_HEIGHT && header->field_7 == 0 &&
        header->field_8 == 0 && header->paletteCount == 0 &&
        std::ranges::all_of(header->palettes, _1 == 0);
}

/**
 * Reads a LOD font, clearing the metrics and offsets of the characters outside `[firstChar, lastChar]` and checking
 * the rest.
 *
 * @tparam Atlas                        Atlas layout of the font, `LodFontAtlas_MM6` or `LodFontAtlas_MMZ`.
 * @param blob                          Font entry.
 * @param[out] header                   Font header.
 * @param[out] atlas                    Font atlas.
 * @param[out] pixels                   Glyph pixel data.
 * @throw Exception                     If a glyph's metrics or pixel data are invalid.
 */
template<class Atlas>
static void decodeFontAs(const Blob &blob, LodFontHeader *header, LodFontAtlas *atlas, Blob *pixels) {
    BlobInputStream stream(blob);
    deserialize(stream, header, tags::via<LodFontHeader_MM6>);
    deserialize(stream, atlas, tags::via<Atlas>);
    *pixels = stream.readAllAsBlob();

    for (int c = 0; c <= 255; c++) {
        if (c < header->firstChar || c > header->lastChar) {
            atlas->metrics[c].width = 0;
            atlas->metrics[c].leftSpacing = 0;
            atlas->metrics[c].rightSpacing = 0;
            atlas->offsets[c] = 0;
            continue;
        }

        const LodFontMetrics &metrics = atlas->metrics[c];
        if (metrics.width < MIN_GLYPH_WIDTH || metrics.width > MAX_GLYPH_WIDTH || metrics.leftSpacing > MAX_GLYPH_SPACING || metrics.rightSpacing > MAX_GLYPH_SPACING) {
            throw Exception("Cannot decode font LOD entry '{}': invalid font metrics encountered for character #{}",
                            blob.displayPath(), c);
        }

        int offset = atlas->offsets[c];
        int size = header->fontHeight * metrics.width;
        if (offset < 0 || size < 0 || size + offset > pixels->size()) {
            throw Exception("Cannot decode font LOD entry '{}': invalid glyph data encountered for character #{}",
                            blob.displayPath(), c);
        }
    }
}

bool lod::detectCompressedDataMm6(const Blob &blob) {
    if (blob.size() < sizeof(LodCompressionHeader_MM6))
        return false;

    MemoryInputStream stream(blob.data(), blob.size());
    LodCompressionHeader_MM6 header;
    deserialize(stream, &header);

    if (header.dataSize != blob.size() - sizeof(LodCompressionHeader_MM6))
        return false;
    if (header.decompressedSize == 0)
        return true; // GrayFace's LOD tools store data that doesn't shrink, or is 256 bytes or less, as is.
    return zlib::detect(blob.subBlob(sizeof(LodCompressionHeader_MM6)));
}

bool lod::detectCompressedDataMm7(const Blob &blob) {
    if (blob.size() < sizeof(LodCompressionHeader_MM7))
        return false;

    MemoryInputStream stream(blob.data(), blob.size());
    LodCompressionHeader_MM7 header;
    deserialize(stream, &header);

    return header.version == 91969 && memcmp(header.signature.data(), "mvii", 4) == 0;
}

template<class Header>
static bool detectCompressedPseudoImageAs(const Blob &blob) {
    if (blob.size() < sizeof(Header))
        return false;

    MemoryInputStream stream(blob.data(), blob.size());
    Header header;
    deserialize(stream, &header);

    return header.size == 0 && header.dataSize > 0 && header.width == 0 && header.height == 0 &&
        header.widthLn2 == 0 && header.heightLn2 == 0 &&
        header.paletteId == 0 && header.anotherPaletteId == 0 && (header.flags & 256) &&
        blob.size() == sizeof(Header) + header.dataSize;
}

bool lod::detectCompressedPseudoImageMm6(const Blob &blob) {
    return detectCompressedPseudoImageAs<LodImageHeader_MM6>(blob);
}

bool lod::detectCompressedPseudoImageMm8(const Blob &blob) {
    return detectCompressedPseudoImageAs<LodImageHeader_MM8>(blob);
}

template<class Header>
static bool detectImageAs(const Blob &blob) {
    if (blob.size() < sizeof(Header))
        return false;

    MemoryInputStream stream(blob.data(), blob.size());
    Header header;
    deserialize(stream, &header);

    return header.size > 0 && header.dataSize > 0 && header.width > 0 && header.height > 0 &&
        header.size == static_cast<uint32_t>(header.width) * header.height &&
        (header.decompressedSize == 0 && header.dataSize >= header.size || header.decompressedSize > 0 && header.decompressedSize >= header.size) &&
        blob.size() == sizeof(Header) + header.dataSize + 0x300;
}

bool lod::detectImageMm6(const Blob &blob) {
    return detectImageAs<LodImageHeader_MM6>(blob);
}

bool lod::detectImageMm8(const Blob &blob) {
    return detectImageAs<LodImageHeader_MM8>(blob);
}

bool lod::detectPaletteMm6(const Blob &blob) {
    if (blob.size() < sizeof(LodImageHeader_MM6))
        return false;

    MemoryInputStream stream(blob.data(), blob.size());
    LodImageHeader_MM6 header;
    deserialize(stream, &header);

    return header.size == 0 && header.dataSize == 0 && header.width == 0 && header.height == 0 &&
        header.widthLn2 == 0 && header.heightLn2 == 0 && header.widthMinus1 == 0 && header.heightMinus1 == 0 &&
        header.anotherPaletteId == 0 && header.decompressedSize == 0 && header.flags == 0 && // MM8's pal857 has its number in paletteId.
        blob.size() == sizeof(LodImageHeader_MM6) + 0x300;
}

bool lod::detectSpriteMm6(const Blob &blob) {
    if (blob.size() < sizeof(LodSpriteHeader_MM6))
        return false;

    MemoryInputStream stream(blob.data(), blob.size());
    LodSpriteHeader_MM6 header;
    deserialize(stream, &header);

    return header.dataSize > 0 && header.width > 0 && header.height > 0 &&
        header.paletteId > 0 && header.unk_0 == 0 && header.emptyBottomLines <= header.height &&
        blob.size() == sizeof(LodSpriteHeader_MM6) + header.height * sizeof(LodSpriteLine_MM6) + header.dataSize;
}

bool lod::detectFontMm6(const Blob &blob) {
    MemoryInputStream stream(blob.data(), blob.size());
    LodFontHeader_MM6 header;
    if (!detectFontHeader(stream, blob.size(), &header) || blob.size() < sizeof(LodFontHeader_MM6) + sizeof(LodFontAtlas_MM6))
        return false;

    LodFontAtlas_MM6 atlas;
    deserialize(stream, &atlas);

    int64_t pixelsSize = 0;
    for (int c = header.firstChar; c <= header.lastChar; c++)
        pixelsSize += static_cast<int64_t>(header.height) * atlas.metrics[c].width;
    size_t expectedSize = sizeof(LodFontHeader_MM6) + sizeof(LodFontAtlas_MM6) + pixelsSize;
    return pixelsSize >= 0 && (blob.size() == expectedSize || blob.size() == expectedSize + 1); // Some Russian MM6 and MM7 fonts have a stray byte after the glyphs.
}

bool lod::detectFontMmz(const Blob &blob) {
    MemoryInputStream stream(blob.data(), blob.size());
    LodFontHeader_MM6 header;
    if (!detectFontHeader(stream, blob.size(), &header) || blob.size() < sizeof(LodFontHeader_MM6) + sizeof(LodFontAtlas_MMZ))
        return false;

    LodFontAtlas_MMZ atlas;
    deserialize(stream, &atlas);

    size_t pixelsSize = 0;
    for (int c = header.firstChar; c <= header.lastChar; c++)
        pixelsSize += static_cast<size_t>(header.height) * atlas.widths[c];
    return blob.size() == sizeof(LodFontHeader_MM6) + sizeof(LodFontAtlas_MMZ) + pixelsSize;
}

template<class Header>
static Blob decodeCompressedAs(const Blob &blob) {
    BlobInputStream stream(blob);
    Header header;
    deserialize(stream, &header);

    Blob result = stream.readAsBlobOrFail(header.dataSize);
    if (header.decompressedSize)
        result = uncompressEntryData(blob, result, header.decompressedSize);
    return result.withDisplayPath(blob.displayPath());
}

Blob lod::decodeCompressedDataMm6(const Blob &blob) {
    if (!detectCompressedDataMm6(blob))
        throw Exception("Cannot decode LOD entry '{}' as LOD compressed data (MM6 format)", blob.displayPath());
    return decodeCompressedAs<LodCompressionHeader_MM6>(blob);
}

Blob lod::decodeCompressedDataMm7(const Blob &blob) {
    if (!detectCompressedDataMm7(blob))
        throw Exception("Cannot decode LOD entry '{}' as LOD compressed data (MM7 format)", blob.displayPath());

    BlobInputStream stream(blob);
    LodCompressionHeader_MM7 header;
    deserialize(stream, &header);

    Blob result;
    if (header.dataSize == blob.size()) {
        // Workaround for a bug in the original LOD writer, where header.dataSize was equal to LOD record size,
        // instead of the size of the data that followed.
        result = stream.readAllAsBlob();
    } else {
        result = stream.readAsBlobOrFail(header.dataSize);
    }
    if (header.decompressedSize)
        result = uncompressEntryData(blob, result, header.decompressedSize);
    return result.withDisplayPath(blob.displayPath());
}

Blob lod::decodeCompressedData(const Blob &blob) {
    if (detectCompressedDataMm7(blob))
        return decodeCompressedDataMm7(blob);
    if (detectCompressedDataMm6(blob))
        return decodeCompressedDataMm6(blob);
    throw Exception("Cannot decode LOD entry '{}' as LOD compressed data", blob.displayPath());
}

Blob lod::decodeCompressedPseudoImageMm6(const Blob &blob) {
    if (!detectCompressedPseudoImageMm6(blob))
        throw Exception("Cannot decode LOD entry '{}' as LOD compressed pseudo image (MM6 format)", blob.displayPath());
    return decodeCompressedAs<LodImageHeader_MM6>(blob);
}

Blob lod::decodeCompressedPseudoImageMm8(const Blob &blob) {
    if (!detectCompressedPseudoImageMm8(blob))
        throw Exception("Cannot decode LOD entry '{}' as LOD compressed pseudo image (MM8 format)", blob.displayPath());
    return decodeCompressedAs<LodImageHeader_MM8>(blob);
}

Blob lod::decodeCompressedPseudoImage(const Blob &blob) {
    if (detectCompressedPseudoImageMm6(blob))
        return decodeCompressedPseudoImageMm6(blob);
    if (detectCompressedPseudoImageMm8(blob))
        return decodeCompressedPseudoImageMm8(blob);
    throw Exception("Cannot decode LOD entry '{}' as LOD compressed pseudo image", blob.displayPath());
}

Blob lod::decodeMaybeCompressed(const Blob &blob) {
    if (detectCompressedDataMm7(blob))
        return decodeCompressedDataMm7(blob);
    if (detectCompressedPseudoImageMm6(blob))
        return decodeCompressedPseudoImageMm6(blob);
    if (detectCompressedPseudoImageMm8(blob))
        return decodeCompressedPseudoImageMm8(blob);
    // MM6 compressed data has no signature, so it goes after the formats with stricter checks.
    if (detectCompressedDataMm6(blob))
        return decodeCompressedDataMm6(blob);
    return Blob::share(blob); // Not compressed.
}

Blob lod::encodeCompressedDataMm7(const Blob &blob) {
    Blob compressed = zlib::compress(blob);

    LodCompressionHeader_MM7 header;
    header.version = 91969;
    header.signature = {{'m', 'v', 'i', 'i'}};
    header.dataSize = compressed.size();
    header.decompressedSize = blob.size();

    return Blob::concat(Blob::view(&header, sizeof(header)), compressed);
}

template<class Header>
static LodImage decodeImageAs(const Blob &blob) {
    BlobInputStream stream(blob);
    Header header;
    deserialize(stream, &header);

    Blob pixels = stream.readAsBlobOrFail(header.dataSize);
    if (header.decompressedSize)
        pixels = uncompressEntryData(blob, pixels, header.decompressedSize);

    assert(pixels.size() >= static_cast<size_t>(header.width) * header.height);

    LodImage result;
    deserialize(stream, &result.palette);
    result.zeroIsTransparent = header.flags & 512;

    // TODO(captainurist): just store blob in GrayscaleImage, no need to copy here.
    result.image = GrayscaleImage::copy(static_cast<const uint8_t *>(pixels.data()), header.width, header.height); // NOLINT: this is not std::copy.
    return result;
}

LodImage lod::decodeImageMm6(const Blob &blob) {
    if (!detectImageMm6(blob))
        throw Exception("Cannot decode LOD entry '{}' as LOD image (MM6 format)", blob.displayPath());
    return decodeImageAs<LodImageHeader_MM6>(blob);
}

LodImage lod::decodeImageMm8(const Blob &blob) {
    if (!detectImageMm8(blob))
        throw Exception("Cannot decode LOD entry '{}' as LOD image (MM8 format)", blob.displayPath());
    return decodeImageAs<LodImageHeader_MM8>(blob);
}

LodImage lod::decodeImage(const Blob &blob) {
    if (detectImageMm6(blob))
        return decodeImageMm6(blob);
    if (detectImageMm8(blob))
        return decodeImageMm8(blob);
    throw Exception("Cannot decode LOD entry '{}' as LOD image", blob.displayPath());
}

template<class Header>
static Sizei decodeSizeAs(const Blob &blob) {
    BlobInputStream stream(blob);
    Header header;
    deserialize(stream, &header);

    return Sizei(header.width, header.height);
}

Sizei lod::decodeImageSizeMm6(const Blob &blob) {
    if (!detectImageMm6(blob))
        throw Exception("Cannot decode LOD entry '{}' as LOD image (MM6 format)", blob.displayPath());
    return decodeSizeAs<LodImageHeader_MM6>(blob);
}

Sizei lod::decodeImageSizeMm8(const Blob &blob) {
    if (!detectImageMm8(blob))
        throw Exception("Cannot decode LOD entry '{}' as LOD image (MM8 format)", blob.displayPath());
    return decodeSizeAs<LodImageHeader_MM8>(blob);
}

Sizei lod::decodeImageSize(const Blob &blob) {
    if (detectImageMm6(blob))
        return decodeImageSizeMm6(blob);
    if (detectImageMm8(blob))
        return decodeImageSizeMm8(blob);
    throw Exception("Cannot decode LOD entry '{}' as LOD image", blob.displayPath());
}

Palette lod::decodePaletteMm6(const Blob &blob) {
    if (!detectPaletteMm6(blob))
        throw Exception("Cannot decode LOD entry '{}' as LOD palette (MM6 format)", blob.displayPath());

    MemoryInputStream stream(blob.data(), blob.size());
    LodImageHeader_MM6 header;
    deserialize(stream, &header);

    Palette result;
    deserialize(stream, &result);
    return result;
}

Palette lod::decodePalette(const Blob &blob) {
    if (detectPaletteMm6(blob))
        return decodePaletteMm6(blob);
    throw Exception("Cannot decode LOD entry '{}' as LOD palette", blob.displayPath());
}

LodSprite lod::decodeSpriteMm6(const Blob &blob) {
    if (!detectSpriteMm6(blob))
        throw Exception("Cannot decode LOD entry '{}' as LOD sprite (MM6 format)", blob.displayPath());

    BlobInputStream stream(blob);
    LodSpriteHeader_MM6 header;
    deserialize(stream, &header);

    std::vector<LodSpriteLine_MM6> lines;
    deserialize(stream, &lines, tags::presized(header.height));

    Blob pixels = stream.readAsBlobOrFail(header.dataSize);
    if (header.decompressedSize)
        pixels = uncompressEntryData(blob, pixels, header.decompressedSize);

    LodSprite result;
    result.paletteId = header.paletteId;
    result.image = GrayscaleImage::solid(0, header.width, header.height);

    for (size_t y = 0; y < header.height; y++) {
        const LodSpriteLine_MM6 &line = lines[y];

        if (line.begin == line.end)
            continue; // Empty line.

        if (line.begin < 0 || line.end < 0 || line.begin > header.width || line.end > header.width || line.begin > line.end ||
            line.offset > pixels.size() || line.offset + line.end - line.begin > pixels.size()) {
            throw Exception("Cannot decode sprite LOD entry '{}': invalid sprite line encountered at y={}",
                            blob.displayPath(), y);
        }

        memcpy(result.image[y].data() + line.begin, static_cast<const char *>(pixels.data()) + line.offset, line.end - line.begin);
    }

    return result;
}

LodSprite lod::decodeSprite(const Blob &blob) {
    if (detectSpriteMm6(blob))
        return decodeSpriteMm6(blob);
    throw Exception("Cannot decode LOD entry '{}' as LOD sprite", blob.displayPath());
}

Sizei lod::decodeSpriteSizeMm6(const Blob &blob) {
    if (!detectSpriteMm6(blob))
        throw Exception("Cannot decode LOD entry '{}' as LOD sprite (MM6 format)", blob.displayPath());
    return decodeSizeAs<LodSpriteHeader_MM6>(blob);
}

Sizei lod::decodeSpriteSize(const Blob &blob) {
    if (detectSpriteMm6(blob))
        return decodeSpriteSizeMm6(blob);
    throw Exception("Cannot decode LOD entry '{}' as LOD sprite", blob.displayPath());
}

LodFont lod::decodeFontMm6(const Blob &blob) {
    if (!detectFontMm6(blob))
        throw Exception("Cannot decode LOD entry '{}' as LOD font (MM6 format)", blob.displayPath());

    LodFont result;
    decodeFontAs<LodFontAtlas_MM6>(blob, &result._header, &result._atlas, &result._pixels);
    return result;
}

LodFont lod::decodeFontMmz(const Blob &blob) {
    if (!detectFontMmz(blob))
        throw Exception("Cannot decode LOD entry '{}' as LOD font (MMZ format)", blob.displayPath());

    LodFont result;
    decodeFontAs<LodFontAtlas_MMZ>(blob, &result._header, &result._atlas, &result._pixels);
    return result;
}

LodFont lod::decodeFont(const Blob &blob) {
    if (detectFontMm6(blob))
        return decodeFontMm6(blob);
    if (detectFontMmz(blob))
        return decodeFontMmz(blob);
    throw Exception("Cannot decode LOD entry '{}' as LOD font", blob.displayPath());
}
