#include "LodFormatEnums.h"

#include "Library/Serialization/EnumSerialization.h"

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(LodFileFormat, CASE_SENSITIVE, {
    {LOD_FILE_COMPRESSED, "compressed_data"},
    {LOD_FILE_IMAGE, "image"},
    {LOD_FILE_PALETTE, "palette"},
    {LOD_FILE_PSEUDO_IMAGE, "pseudo_image_data"},
    {LOD_FILE_SPRITE, "sprite"},
    {LOD_FILE_FONT, "font"},
    {LOD_FILE_RAW, "raw_data"},
})
