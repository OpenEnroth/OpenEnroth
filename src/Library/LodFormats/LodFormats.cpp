#include "LodFormats.h"

#include <optional>
#include <span>
#include <vector>
#include <string>

#include "LodFormatSnapshots.h"

#include "Library/Binary/ContainerSerialization.h"
#include "Library/Snapshots/CommonSnapshots.h"
#include "Library/Compression/Compression.h"
#include "Library/Serialization/EnumSerialization.h"

#include "Utility/Streams/MemoryInputStream.h"
#include "Utility/Streams/BlobInputStream.h"
#include "Utility/Memory/Blob.h"
#include "Utility/String/Ascii.h"
#include "Utility/Exception.h"

static void deserialize(InputStream &src, Palette *dst) {
    std::array<std::uint8_t, 0x300> rawPalette;
    src.readOrFail(rawPalette.data(), rawPalette.size());

    for (size_t i = 0; i < 256; i++)
        dst->colors[i] = Color(rawPalette[i * 3], rawPalette[i * 3 + 1], rawPalette[i * 3 + 2]);
}

static std::optional<LodFileFormat> checkCompressed(const Blob &blob) {
    if (blob.size() < sizeof(LodCompressionHeader_MM6))
        return {};

    MemoryInputStream stream(blob.data(), blob.size());
    LodCompressionHeader_MM6 header;
    deserialize(stream, &header);

    if (header.version == 91969 && memcmp(header.signature.data(), "mvii", 4) == 0)
        return LOD_FILE_COMPRESSED; // We don't check that sizes match here, we'll throw if they don't on read.

    return {};
}

static std::optional<LodFileFormat> checkImage(const Blob &blob, std::string_view fileName) {
    if (blob.size() < sizeof(LodImageHeader_MM6))
        return {};

    MemoryInputStream stream(blob.data(), blob.size());
    LodImageHeader_MM6 header;
    deserialize(stream, &header);

    std::string name;
    reconstruct(header.name, &name);
    if (!fileName.empty() && ascii::toLower(name) != ascii::toLower(fileName))
        return {};

    if (header.size == 0 && header.dataSize == 0 && header.width == 0 && header.height == 0 &&
        header.widthLn2 == 0 && header.heightLn2 == 0 && header.widthMinus1 == 0 && header.heightMinus1 == 0 &&
        header.paletteId == 0 && header.anotherPaletteId == 0 && header.decompressedSize == 0 && header.flags == 0 &&
        blob.size() == sizeof(LodImageHeader_MM6) + 0x300)
        return LOD_FILE_PALETTE;

    if (header.size == 0 && header.dataSize > 0 && header.width == 0 && header.height == 0 &&
        header.widthLn2 == 0 && header.heightLn2 == 0 &&
        header.paletteId == 0 && header.anotherPaletteId == 0 && (header.flags & 256) &&
        blob.size() == sizeof(LodImageHeader_MM6) + header.dataSize)
        return LOD_FILE_PSEUDO_IMAGE;

    if (header.size > 0 && header.dataSize > 0 && header.width > 0 && header.height > 0 &&
        header.size == header.width * header.height &&
        (header.decompressedSize == 0 && header.dataSize >= header.size || header.decompressedSize > 0 && header.decompressedSize >= header.size) &&
        blob.size() == sizeof(LodImageHeader_MM6) + header.dataSize + 0x300)
        return LOD_FILE_IMAGE;

    return {};
}

static std::optional<LodFileFormat> checkSprite(const Blob &blob, std::string_view fileName) {
    if (blob.size() < sizeof(LodSpriteHeader_MM6))
        return {};

    MemoryInputStream stream(blob.data(), blob.size());
    LodSpriteHeader_MM6 header;
    deserialize(stream, &header);

    std::string name;
    reconstruct(header.name, &name);
    if (!fileName.empty() && ascii::toLower(name) != ascii::toLower(fileName))
        return {};

    if (header.dataSize > 0 && header.width > 0 && header.height > 0 &&
        header.paletteId > 0 && header.unk_0 == 0 && header.emptyBottomLines <= header.height &&
        blob.size() == sizeof(LodSpriteHeader_MM6) + header.height * sizeof(LodSpriteLine_MM6) + header.dataSize)
        return LOD_FILE_SPRITE;

    return {};
}

LodFileFormat lod::magic(const Blob &blob, std::string_view fileName) {
    if (auto result = checkCompressed(blob))
        return *result;
    if (auto result = checkImage(blob, fileName))
        return *result;
    if (auto result = checkSprite(blob, fileName))
        return *result;
    return LOD_FILE_RAW;
}

Blob lod::decodeCompressed(const Blob &blob) {
    LodFileFormat format = magic(blob, {});
    if (format == LOD_FILE_RAW)
        return Blob::share(blob); // Not compressed.

    if (format == LOD_FILE_COMPRESSED) {
        BlobInputStream stream(blob);
        LodCompressionHeader_MM6 header;
        deserialize(stream, &header);

        Blob result;
        if (header.dataSize == blob.size()) {
            // Workaround for a bug in the original LOD writer, where header.dataSize was equal to LOD record size,
            // instead of the size of the data that followed.
            result = stream.tail();
        } else {
            result = stream.readBlobOrFail(header.dataSize);
        }
        if (header.decompressedSize)
            result = zlib::uncompress(result, header.decompressedSize);
        return result;
    }

    if (format == LodFileFormat::LOD_FILE_PSEUDO_IMAGE) {
        BlobInputStream stream(blob);
        LodImageHeader_MM6 header;
        deserialize(stream, &header);

        Blob result = stream.readBlobOrFail(header.dataSize);
        if (header.decompressedSize)
            result = zlib::uncompress(result, header.decompressedSize);
        return result;
    }

    throw Exception("Cannot uncompress LOD entry of type '{}', operation is not supported", toString(format));
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
    LodFileFormat format = magic(blob, {});
    if (format != LOD_FILE_PALETTE && format != LOD_FILE_IMAGE)
        throw Exception("Cannot decode LOD entry of type '{}' as '{}'", toString(format), toString(LOD_FILE_PALETTE));

    MemoryInputStream stream(blob.data(), blob.size());
    LodImageHeader_MM6 header;
    deserialize(stream, &header);

    stream.skipOrFail(header.dataSize);

    Palette result;
    deserialize(stream, &result);
    return result;
}

LodImage lod::decodeImage(const Blob &blob) {
    LodFileFormat format = magic(blob, {});
    if (format != LOD_FILE_IMAGE && format != LOD_FILE_PALETTE) {
        format = magic(blob, {});
        throw Exception("Cannot decode LOD entry of type '{}' as '{}'", toString(format), toString(LOD_FILE_IMAGE));
    }

    BlobInputStream stream(blob);
    LodImageHeader_MM6 header;
    deserialize(stream, &header);

    Blob pixels;
    if (format == LOD_FILE_IMAGE) {
        pixels = stream.readBlobOrFail(header.dataSize);
        if (header.decompressedSize)
            pixels = zlib::uncompress(pixels, header.decompressedSize);

        // Note that this check isn't redundant. The checks in magic() only check sizes as written in the header.
        // Actual stream size might be different.
        if (pixels.size() < header.width * header.height)
            throw Exception("Cannot decode LOD entry of type '{}': expected {}x{}={} pixels, got {}",
                            toString(LOD_FILE_IMAGE), header.width, header.height, header.width * header.height, pixels.size());
    }

    LodImage result;
    deserialize(stream, &result.palette);
    result.zeroIsTransparent = header.flags & 512;

    // TODO(captainurist): just store blob in GrayscaleImage, no need to copy here.
    if (pixels)
        result.image = GrayscaleImage::copy(header.width, header.height, static_cast<const uint8_t *>(pixels.data())); // NOLINT: this is not std::copy.
    return result;
}

LodSprite lod::decodeSprite(const Blob &blob) {
    LodFileFormat format = magic(blob, {});
    if (format != LOD_FILE_SPRITE)
        throw Exception("Cannot decode LOD entry of type '{}' as '{}'", toString(format), toString(LOD_FILE_SPRITE));

    BlobInputStream stream(blob);
    LodSpriteHeader_MM6 header;
    deserialize(stream, &header);

    std::vector<LodSpriteLine_MM6> lines;
    deserialize(stream, &lines, tags::presized(header.height));

    Blob pixels = stream.readBlobOrFail(header.dataSize);
    if (header.decompressedSize)
        pixels = zlib::uncompress(pixels, header.decompressedSize);

    LodSprite result;
    result.paletteId = header.paletteId;
    result.image = GrayscaleImage::solid(header.width, header.height, 0);

    for (size_t y = 0; y < header.height; y++) {
        const LodSpriteLine_MM6 &line = lines[y];

        if (line.begin == line.end)
            continue; // Empty line.

        if (line.begin < 0 || line.end < 0 || line.begin > header.width || line.end > header.width || line.begin > line.end ||
            line.offset > pixels.size() || line.offset + line.end - line.begin > pixels.size())
            throw Exception("Cannot decode LOD entry of type '{}': invalid sprite line encountered at y={}", toString(LOD_FILE_SPRITE), y);

        memcpy(result.image[y].data() + line.begin, static_cast<const char *>(pixels.data()) + line.offset, line.end - line.begin);
    }

    return result;
}
