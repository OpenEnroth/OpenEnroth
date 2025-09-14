#include "Magic.h"

#include <bit>

#include "Library/Lod/LodReader.h"
#include "Library/Snd/SndReader.h"
#include "Library/Vid/VidReader.h"
#include "Library/LodFormats/LodFormats.h"
#include "Library/Image/Pcx.h"
#include "Library/Image/Png.h"
#include "Library/Binary/ContainerSerialization.h"
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
    return MAGIC_UNRECOGNIZED;
}
