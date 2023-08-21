#include "MapEnums.h"

#include "Library/Serialization/EnumSerialization.h"
#include "Library/Serialization/EnumSerializer.h"

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(MapStartPoint, CASE_SENSITIVE, {
    {MapStartPoint_Party, "Party Start"},
    {MapStartPoint_North, "North Start"},
    {MapStartPoint_South, "South Start"},
    {MapStartPoint_East, "East Start"},
    {MapStartPoint_West, "West Start"}
})
