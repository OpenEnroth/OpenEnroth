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
        return "LOD compressed data (MM7 format)";
    case MAGIC_LOD_COMPRESSED_PSEUDO_IMAGE_MM6:
        return "LOD compressed pseudo image (MM6 format)";
    case MAGIC_LOD_IMAGE_MM6:
        return "LOD image (MM6 format)";
    case MAGIC_LOD_PALETTE_MM6:
        return "LOD palette (MM6 format)";
    case MAGIC_LOD_SPRITE_MM6:
        return "LOD sprite (MM6 format)";
    case MAGIC_LOD_FONT_MM6:
        return "LOD font (MM6 format)";
    case MAGIC_LOD_FONT_MMX:
        return "LOD font (MMZ format)";
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
