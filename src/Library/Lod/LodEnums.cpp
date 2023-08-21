#include "LodEnums.h"

#include "Library/Serialization/EnumSerialization.h"
#include "Library/Serialization/EnumSerializer.h"

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(LodVersion, CASE_SENSITIVE, {
    {LOD_VERSION_MM6,       "MMVI"},
    {LOD_VERSION_MM6_GAME,  "GameMMVI"},
    {LOD_VERSION_MM7,       "MMVII"},
    {LOD_VERSION_MM8,       "MMVIII"},
})
