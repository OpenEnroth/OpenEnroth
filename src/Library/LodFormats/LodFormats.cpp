#include "LodFormats.h"

#include <optional>
#include <span>
#include <utility>
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

bool lod::detectCompressedData(const Blob &blob) {
    if (blob.size() < sizeof(LodCompressionHeader_MM6))
        return false;

    MemoryInputStream stream(blob.data(), blob.size());
    LodCompressionHeader_MM6 header;
    deserialize(stream, &header);

    return header.version == 91969 && memcmp(header.signature.data(), "mvii", 4) == 0;
}

bool lod::detectCompressedPseudoImage(const Blob &blob) {
    if (blob.size() < sizeof(LodImageHeader_MM6))
        return false;

    MemoryInputStream stream(blob.data(), blob.size());
    LodImageHeader_MM6 header;
    deserialize(stream, &header);

    return header.size == 0 && header.dataSize > 0 && header.width == 0 && header.height == 0 &&
        header.widthLn2 == 0 && header.heightLn2 == 0 &&
        header.paletteId == 0 && header.anotherPaletteId == 0 && (header.flags & 256) &&
        blob.size() == sizeof(LodImageHeader_MM6) + header.dataSize;
}

bool lod::detectImage(const Blob &blob, bool *isPalette) {
    if (isPalette)
        *isPalette = false;

    if (blob.size() < sizeof(LodImageHeader_MM6))
        return false;

    MemoryInputStream stream(blob.data(), blob.size());
    LodImageHeader_MM6 header;
    deserialize(stream, &header);

    if (header.size == 0 && header.dataSize == 0 && header.width == 0 && header.height == 0 &&
        header.widthLn2 == 0 && header.heightLn2 == 0 && header.widthMinus1 == 0 && header.heightMinus1 == 0 &&
        header.paletteId == 0 && header.anotherPaletteId == 0 && header.decompressedSize == 0 && header.flags == 0 &&
        blob.size() == sizeof(LodImageHeader_MM6) + 0x300)
        return isPalette ? *isPalette = true : true;

    if (header.size > 0 && header.dataSize > 0 && header.width > 0 && header.height > 0 &&
        header.size == header.width * header.height &&
        (header.decompressedSize == 0 && header.dataSize >= header.size || header.decompressedSize > 0 && header.decompressedSize >= header.size) &&
        blob.size() == sizeof(LodImageHeader_MM6) + header.dataSize + 0x300)
        return true;

    return false;
}

bool lod::detectSprite(const Blob &blob) {
    if (blob.size() < sizeof(LodSpriteHeader_MM6))
        return false;

    MemoryInputStream stream(blob.data(), blob.size());
    LodSpriteHeader_MM6 header;
    deserialize(stream, &header);

    if (header.dataSize > 0 && header.width > 0 && header.height > 0 &&
        header.paletteId > 0 && header.unk_0 == 0 && header.emptyBottomLines <= header.height &&
        blob.size() == sizeof(LodSpriteHeader_MM6) + header.height * sizeof(LodSpriteLine_MM6) + header.dataSize)
        return true;

    return false;
}

bool lod::detectFont(const Blob &blob) {
    if (blob.size() < sizeof(LodFontHeader_MM7) + std::min(sizeof(LodFontAtlas_MM7), sizeof(LodFontAtlas_MMX)))
        return {};

    MemoryInputStream stream(blob.data(), blob.size());
    LodFontHeader_MM7 header;
    deserialize(stream, &header);

    if (header.firstChar < header.lastChar && header.field_3 == 8 && header.field_4 == 0 && header.field_5 == 0 &&
        header.height >= MIN_GLYPH_HEIGHT && header.height <= MAX_GLYPH_HEIGHT && header.field_7 == 0 &&
        header.field_8 == 0 && header.paletteCount == 0 &&
        std::ranges::all_of(header.palettes, _1 == 0))
        return true;

    return false;
}

Blob lod::decodeCompressedData(const Blob &blob) {
    if (!detectCompressedData(blob))
        throw Exception("Cannot decode LOD entry '{}' as LOD compressed data", blob.displayPath());

    BlobInputStream stream(blob);
    LodCompressionHeader_MM6 header;
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
        result = zlib::uncompress(result.withDisplayPath(blob.displayPath()), header.decompressedSize);
    return result.withDisplayPath(blob.displayPath());
}

Blob lod::decodeCompressedPseudoImage(const Blob &blob) {
    if (!detectCompressedPseudoImage(blob))
        throw Exception("Cannot decode LOD entry '{}' as LOD compressed pseudo image", blob.displayPath());

    BlobInputStream stream(blob);
    LodImageHeader_MM6 header;
    deserialize(stream, &header);

    Blob result = stream.readAsBlobOrFail(header.dataSize);
    if (header.decompressedSize)
        result = zlib::uncompress(result.withDisplayPath(blob.displayPath()), header.decompressedSize);
    return result.withDisplayPath(blob.displayPath());
}

Blob lod::decodeMaybeCompressed(const Blob &blob) {
    if (detectCompressedData(blob))
        return decodeCompressedData(blob);

    if (detectCompressedPseudoImage(blob))
        return decodeCompressedPseudoImage(blob);

    return Blob::share(blob); // Not compressed.
}

Blob lod::encodeCompressed(const Blob &blob) {
    Blob compressed = zlib::compress(blob);

    LodCompressionHeader_MM6 header;
    header.version = 91969;
    header.signature = {{'m', 'v', 'i', 'i'}};
    header.dataSize = compressed.size();
    header.decompressedSize = blob.size();

    return Blob::concat(Blob::view(&header, sizeof(header)), compressed);
}

Palette lod::decodePalette(const Blob &blob) {
    if (!detectImage(blob))
        throw Exception("Cannot decode LOD entry '{}' as LOD palette", blob.displayPath());

    MemoryInputStream stream(blob.data(), blob.size());
    LodImageHeader_MM6 header;
    deserialize(stream, &header);

    stream.skipOrFail(header.dataSize);

    Palette result;
    deserialize(stream, &result);
    return result;
}

LodImage lod::decodeImage(const Blob &blob) {
    bool isPalette = false;
    if (!detectImage(blob, &isPalette))
        throw Exception("Cannot decode LOD entry '{}' as LOD image", blob.displayPath());

    BlobInputStream stream(blob);
    LodImageHeader_MM6 header;
    deserialize(stream, &header);

    Blob pixels;
    if (!isPalette) {
        pixels = stream.readAsBlobOrFail(header.dataSize);
        if (header.decompressedSize)
            pixels = zlib::uncompress(pixels.withDisplayPath(blob.displayPath()), header.decompressedSize);

        // Note that this check isn't redundant. The checks in magic() only check sizes as written in the header.
        // Actual stream size might be different.
        if (pixels.size() < header.width * header.height)
            throw Exception("Cannot decode image LOD entry '{}': expected {}x{}={} pixels, got {}",
                            blob.displayPath(), header.width, header.height,
                            header.width * header.height, pixels.size());
    }

    LodImage result;
    deserialize(stream, &result.palette);
    result.zeroIsTransparent = header.flags & 512;

    // TODO(captainurist): just store blob in GrayscaleImage, no need to copy here.
    if (pixels)
        result.image = GrayscaleImage::copy(static_cast<const uint8_t *>(pixels.data()), header.width, header.height); // NOLINT: this is not std::copy.
    return result;
}

Sizei lod::decodeImageSize(const Blob &blob) {
    if (!detectImage(blob))
        throw Exception("Cannot decode LOD entry '{}' as LOD image", blob.displayPath());

    BlobInputStream stream(blob);
    LodImageHeader_MM6 header;
    deserialize(stream, &header);

    return Sizei(header.width, header.height);
}

LodSprite lod::decodeSprite(const Blob &blob) {
    if (!detectSprite(blob))
        throw Exception("Cannot decode LOD entry '{}' as LOD sprite", blob.displayPath());

    BlobInputStream stream(blob);
    LodSpriteHeader_MM6 header;
    deserialize(stream, &header);

    std::vector<LodSpriteLine_MM6> lines;
    deserialize(stream, &lines, tags::presized(header.height));

    Blob pixels = stream.readAsBlobOrFail(header.dataSize);
    if (header.decompressedSize)
        pixels = zlib::uncompress(pixels.withDisplayPath(blob.displayPath()), header.decompressedSize);

    LodSprite result;
    result.paletteId = header.paletteId;
    result.image = GrayscaleImage::solid(0, header.width, header.height);

    for (size_t y = 0; y < header.height; y++) {
        const LodSpriteLine_MM6 &line = lines[y];

        if (line.begin == line.end)
            continue; // Empty line.

        if (line.begin < 0 || line.end < 0 || line.begin > header.width || line.end > header.width || line.begin > line.end ||
            line.offset > pixels.size() || line.offset + line.end - line.begin > pixels.size())
            throw Exception("Cannot decode sprite LOD entry '{}': invalid sprite line encountered at y={}",
                            blob.displayPath(), y);

        memcpy(result.image[y].data() + line.begin, static_cast<const char *>(pixels.data()) + line.offset, line.end - line.begin);
    }

    return result;
}

LodFont lod::decodeFont(const Blob &blob) {
    if (!detectFont(blob))
        throw Exception("Cannot decode LOD entry '{}' as LOD font", blob.displayPath());

    LodFontHeader_MM7 header;
    LodFontAtlas atlas;
    Blob pixels;

    auto decodeFontVia = [&](auto via) {
        BlobInputStream stream(blob);
        deserialize(stream, &header);
        deserialize(stream, &atlas, via);
        pixels = stream.readAllAsBlob();

        for (int c = header.firstChar; c <= header.lastChar; c++) {
            // Check that font metrics are sane.
            const LodFontMetrics &metrics = atlas.metrics[c];
            if (metrics.width < MIN_GLYPH_WIDTH || metrics.width > MAX_GLYPH_WIDTH ||
                metrics.leftSpacing > MAX_GLYPH_SPACING || metrics.rightSpacing > MAX_GLYPH_SPACING)
                throw Exception("Cannot decode font LOD entry '{}': invalid font metrics encountered for character #{}",
                                blob.displayPath(), c);

            // Check that all offsets point into the pixel data.
            int size = header.height * metrics.width;
            if (atlas.offsets[c] < 0 || size + atlas.offsets[c] > pixels.size())
                throw Exception("Cannot decode font LOD entry '{}': invalid glyph data encountered for character #{}",
                                blob.displayPath(), c);
        }
    };

    try {
        decodeFontVia(tags::via<LodFontAtlas_MM7>);
    } catch (const std::exception &e) {
        try {
            decodeFontVia(tags::via<LodFontAtlas_MMX>);
        } catch (const std::exception &) {
            throw e; // Re-throw MM7 exception if trying both formats failed.
        }
    }

    // Characters outside `[firstChar, lastChar]` are garbage in the original files. Zero them out so that `LodFont`
    // reports them as unsupported.
    for (int c = 0; c <= 255; c++) {
        if (c < header.firstChar || c > header.lastChar) {
            atlas.metrics[c] = {};
            atlas.offsets[c] = 0;
        }
    }

    return LodFont(header.height, atlas, std::move(pixels));
}
