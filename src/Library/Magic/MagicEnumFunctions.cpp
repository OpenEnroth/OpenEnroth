#include "MagicEnumFunctions.h"

#include <cassert>

std::string_view formatDescription(MagicFileFormat format) {
    switch (format) {
    default:
        assert(false);
        [[fallthrough]];
    case MAGIC_UNRECOGNIZED:
        return "Raw data";
    case MAGIC_LOD:
        return "LOD archive";
    case MAGIC_LOD_COMPRESSED_DATA:
        return "LOD compressed file";
    case MAGIC_LOD_COMPRESSED_PSEUDO_IMAGE:
        return "LOD compressed pseudo image";
    case MAGIC_LOD_IMAGE:
        return "LOD image";
    case MAGIC_LOD_PALETTE:
        return "LOD palette";
    case MAGIC_LOD_SPRITE:
        return "LOD sprite";
    case MAGIC_LOD_FONT:
        return "LOD font";
    case MAGIC_VID:
        return "VID archive";
    case MAGIC_SND:
        return "SND archive";
    case MAGIC_PNG:
        return "PNG image";
    case MAGIC_PCX:
        return "PCX image";
    case MAGIC_WAV:
        return "WAV file";
    }
}
