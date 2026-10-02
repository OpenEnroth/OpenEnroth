#include "ResourceMaskTable.h"

#include "Library/Json/Json.h"

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(ResourceMaskTable, (
    (bitmaps, "bitmaps"),
    (icons, "icons")
))
