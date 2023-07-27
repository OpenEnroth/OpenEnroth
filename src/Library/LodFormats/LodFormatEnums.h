#pragma once

#include "Utility/Flags.h"

enum class LodFileFormat {
    LOD_FILE_COMPRESSED, // Potentially compressed file, `LodCompressionHeader_MM6` for header.
    LOD_FILE_IMAGE, // Image file, `LodImageHeader_MM6` for header, ends with a palette.
    LOD_FILE_PALETTE, // Palette file, `LodImageHeader_MM6` for header, contains no pixels.
    LOD_FILE_PSEUDO_IMAGE, // Potentially compressed file, `LodImageHeader_MM6` for header, but not an image. No palette at the end.
    LOD_FILE_SPRITE, // Sprite file, `LodSpriteHeader_MM6` for header.
    LOD_FILE_RAW, // None of the above.
};
using enum LodFileFormat;


enum class LodDecodeFlag {
    LOD_ALLOW_BORKED_COMPRESSED_SIZE = 0x1,
};
using enum LodDecodeFlag;
MM_DECLARE_FLAGS(LodDecodeFlags, LodDecodeFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(LodDecodeFlags)
