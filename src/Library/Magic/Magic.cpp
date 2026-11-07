#include "Magic.h"

#include <bit>

#include "Library/Lod/LodReader.h"
#include "Library/Snd/SndReader.h"
#include "Library/Vid/VidReader.h"
#include "Library/LodFormats/LodFormats.h"
#include "Library/Image/Pcx.h"
#include "Library/Image/Png.h"
#include "Library/Binary/CommonSerialization.h"
#include "Utility/Streams/BlobInputStream.h"

namespace wav {
bool detect(const Blob &data) {
    if (data.size() < 12)
        return false; // Need at least the RIFF header (12 bytes): "RIFF" + ChunkSize + "WAVE".

    BlobInputStream stream(data);
    std::array<char, 4> riff;
    uint32_t chunkSize;
    std::array<char, 4> form;
    deserialize(stream, &riff);
    deserialize(stream, &chunkSize);
    deserialize(stream, &form);

    if (std::string_view(form.data(), form.size()) != "WAVE")
        return false;

    std::string_view riffStr = std::string_view(riff.data(), riff.size());
    if (riffStr == "RIFF") {
        if (chunkSize != data.size() - 8)
            return false;
    } else if (riffStr == "RIFX") { // big-endian RIFF.
        chunkSize = std::byteswap(chunkSize);
        if (chunkSize != data.size() - 8)
            return false;
    } else if (riffStr == "RF64") { // 64-bit RIFF (little-endian), root ChunkSize must be 0xFFFFFFFF.
        if (chunkSize != 0xFFFFFFFFu)
            return false;
    } else {
        return false; // Fourcc mismatch.
    }

    return true;
}
} // namespace wav

namespace smk {
bool detect(const Blob &data) {
    if (data.size() < 104)
        return false; // Smacker header is 104 bytes.

    BlobInputStream stream(data);
    std::array<char, 4> signature;
    uint32_t width;
    uint32_t height;
    deserialize(stream, &signature);
    deserialize(stream, &width);
    deserialize(stream, &height);

    std::string_view signatureStr = std::string_view(signature.data(), signature.size());
    return (signatureStr == "SMK2" || signatureStr == "SMK4") && width > 0 && height > 0;
}
} // namespace smk

namespace bik {
bool detect(const Blob &data) {
    if (data.size() < 44)
        return false; // Bink header is 44 bytes.

    BlobInputStream stream(data);
    std::array<char, 4> signature;
    uint32_t size;
    deserialize(stream, &signature);
    deserialize(stream, &size);

    std::string_view signatureStr = std::string_view(signature.data(), 3);
    char version = signature[3];
    bool isBink1 = signatureStr == "BIK" && std::string_view("bfghik").contains(version); // Version letters FFmpeg accepts.
    bool isBink2 = signatureStr == "KB2" && std::string_view("adfghijk").contains(version);
    return (isBink1 || isBink2) && size == data.size() - 8;
}
} // namespace bik

MagicFileFormat magic(const Blob &data) {
    if (lod::detect(data))
        return MAGIC_LOD;
    if (lod::detectCompressedData(data))
        return MAGIC_LOD_COMPRESSED_DATA;
    if (lod::detectCompressedPseudoImage(data))
        return MAGIC_LOD_COMPRESSED_PSEUDO_IMAGE;
    bool isPalette = false;
    if (lod::detectImage(data, &isPalette))
        return isPalette ? MAGIC_LOD_PALETTE : MAGIC_LOD_IMAGE;
    if (lod::detectSprite(data))
        return MAGIC_LOD_SPRITE;
    if (lod::detectFont(data))
        return MAGIC_LOD_FONT;
    if (vid::detect(data))
        return MAGIC_VID;
    if (snd::detect(data))
        return MAGIC_SND;
    if (png::detect(data))
        return MAGIC_PNG;
    if (pcx::detect(data))
        return MAGIC_PCX;
    if (wav::detect(data))
        return MAGIC_WAV;
    if (smk::detect(data))
        return MAGIC_SMK;
    if (bik::detect(data))
        return MAGIC_BIK;
    return MAGIC_UNRECOGNIZED;
}
