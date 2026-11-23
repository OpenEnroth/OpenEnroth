#include "MagicEnumFunctions.h"

#include <cassert>

std::string_view formatDescription(MagicFileFormat format) {
    switch (format) {
    default:
        assert(false);
        [[fallthrough]];
    case MAGIC_UNRECOGNIZED:
        return "raw data";
    case MAGIC_LOD:
        return "LOD archive";
    case MAGIC_LOD_COMPRESSED_DATA_MM7:
        return "MM7 LOD compressed file";
    case MAGIC_LOD_COMPRESSED_PSEUDO_IMAGE_MM6:
        return "MM6 LOD compressed pseudo image";
    case MAGIC_LOD_IMAGE_MM6:
        return "MM6 LOD image";
    case MAGIC_LOD_PALETTE_MM6:
        return "MM6 LOD palette";
    case MAGIC_LOD_SPRITE_MM6:
        return "MM6 LOD sprite";
    case MAGIC_LOD_FONT_MM6:
        return "MM6 LOD font";
    case MAGIC_LOD_FONT_MMX:
        return "MMX LOD font";
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
    case MAGIC_SMK:
        return "Smacker video";
    case MAGIC_BIK:
        return "Bink video";
    }
}
