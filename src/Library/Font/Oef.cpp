#include "Oef.h"

#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <span>
#include <string_view>
#include <vector>

#include "OefSnapshots.h"

#include "Library/Binary/CommonSerialization.h"
#include "Library/Compression/Compression.h"
#include "Library/Image/Image.h"
#include "Library/Snapshots/CommonSnapshots.h"

#include "Utility/Exception.h"
#include "Utility/Memory/Blob.h"
#include "Utility/Streams/BlobInputStream.h"
#include "Utility/Streams/BlobOutputStream.h"
#include "Utility/Streams/MemoryInputStream.h"

namespace {

constexpr std::string_view OEF_SIGNATURE = "OEFT";
constexpr uint32_t OEF_VERSION = 1;

// Sanity limits, so that malformed data is rejected instead of triggering huge allocations.
constexpr uint32_t MAX_FONT_HEIGHT = 256;
constexpr int32_t MAX_GLYPH_WIDTH = 256;
constexpr int32_t MAX_GLYPH_SPACING = 256;
constexpr uint32_t MAX_CODE_POINT = 0x110000;

} // namespace

bool oef::detect(const Blob &blob) {
    if (blob.size() < sizeof(OefHeader))
        return false;

    MemoryInputStream stream(blob.data(), blob.size());
    OefHeader header;
    deserialize(stream, &header);

    return std::string_view(header.signature.data(), header.signature.size()) == OEF_SIGNATURE &&
           header.version == OEF_VERSION;
}

Font oef::decode(const Blob &blob) {
    if (!detect(blob))
        throw Exception("Cannot decode entry '{}' as an OpenEnroth font", blob.displayPath());

    BlobInputStream stream(blob);
    OefHeader header;
    deserialize(stream, &header);

    if (header.height == 0 || header.height > MAX_FONT_HEIGHT)
        throw Exception("Cannot decode font '{}': invalid font height {}", blob.displayPath(), header.height);
    if (header.glyphCount > MAX_CODE_POINT)
        throw Exception("Cannot decode font '{}': invalid glyph count {}", blob.displayPath(), header.glyphCount);

    Blob payload = zlib::uncompress(stream.readAllAsBlob(), header.decompressedSize);
    if (payload.size() != header.decompressedSize)
        throw Exception("Cannot decode font '{}': expected {} bytes of font data, got {}", blob.displayPath(),
                        header.decompressedSize, payload.size());

    BlobInputStream payloadStream(payload);
    std::vector<OefGlyph> glyphs(header.glyphCount);
    for (OefGlyph &glyph : glyphs)
        deserialize(payloadStream, &glyph);

    Blob pixels = payloadStream.readAllAsBlob();

    Font result(header.height);
    for (const OefGlyph &glyph : glyphs) {
        if (glyph.codePoint == 0 || glyph.codePoint >= MAX_CODE_POINT)
            throw Exception("Cannot decode font '{}': invalid code point {} in glyph table", blob.displayPath(),
                            glyph.codePoint);
        if (glyph.width < 0 || glyph.width > MAX_GLYPH_WIDTH ||
            std::abs(glyph.leftSpacing) > MAX_GLYPH_SPACING || std::abs(glyph.rightSpacing) > MAX_GLYPH_SPACING)
            throw Exception("Cannot decode font '{}': invalid glyph metrics for code point {}", blob.displayPath(),
                            glyph.codePoint);

        size_t size = static_cast<size_t>(glyph.width) * header.height;
        if (glyph.pixelOffset + size > pixels.size())
            throw Exception("Cannot decode font '{}': invalid glyph data for code point {}", blob.displayPath(),
                            glyph.codePoint);

        GrayscaleImageView image(static_cast<const uint8_t *>(pixels.data()) + glyph.pixelOffset, glyph.width,
                                 header.height);
        result.add(glyph.codePoint, GlyphMetrics(glyph.leftSpacing, glyph.width, glyph.rightSpacing), image);
    }

    return result;
}

Blob oef::encode(const Font &font) {
    assert(font); // Encoding an invalid font is a coding error.

    Blob payload;
    BlobOutputStream payloadStream(&payload);

    uint32_t pixelOffset = 0;
    for (int i = 0; i < font.size(); i++) {
        const GlyphMetrics &metrics = font.metrics(i);

        OefGlyph glyph;
        glyph.codePoint = font.character(i);
        glyph.leftSpacing = metrics.leftSpacing;
        glyph.width = metrics.width;
        glyph.rightSpacing = metrics.rightSpacing;
        glyph.pixelOffset = pixelOffset;
        serialize(glyph, &payloadStream);

        pixelOffset += metrics.width * font.height();
    }

    for (int i = 0; i < font.size(); i++) {
        std::span<const uint8_t> pixels = font.image(i).pixels();
        payloadStream.write(pixels.data(), pixels.size());
    }

    payloadStream.close(); // Need to close before we can compress `payload`.

    Blob result;
    BlobOutputStream stream(&result);

    OefHeader header;
    std::ranges::copy(OEF_SIGNATURE, header.signature.begin());
    header.version = OEF_VERSION;
    header.decompressedSize = payload.size();
    header.height = font.height();
    header.glyphCount = font.size();
    serialize(header, &stream);

    stream.write(zlib::compress(payload));
    stream.close();
    return result;
}
